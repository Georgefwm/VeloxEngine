#include "Entity.h"
#include <PCH.h>

#include <glm/gtx/rotate_vector.hpp>
#include <utility>

static Velox::EntityManager s_entityManager;

void Velox::initEntitySystem()
{
    s_entityManager = Velox::EntityManager();
}

Velox::EntityManager* Velox::getEntityManager()
{
    return &s_entityManager;
}

void Velox::Entity::update(double& getDeltaTime)
{
    if (updateFunction == nullptr)
        return;
    
    updateFunction(*this, getDeltaTime);
}

void Velox::Entity::update(double& getDeltaTime, Velox::Entity* parentRef)
{
    if (parentRef != nullptr)
    {
        // GM: This is jank, sorry :(
        absolutePosition = vec3(vec2(parentRef->absolutePosition) +
            glm::rotate(vec2(position), glm::radians(parentRef->absoluteRotation)), 0.0f);

        absoluteRotation = glm::mod(parentRef->absoluteRotation + rotation, 360.0f);
        absoluteScale    = parentRef->absoluteScale * scale;
    }
    else
    {
        absolutePosition = position;
        absoluteRotation = glm::mod(rotation, 360.0f);
        absoluteScale    = scale;
    }

    if (updateFunction == nullptr)
        return;
    
    updateFunction(*this, getDeltaTime);
}

void Velox::Entity::draw()
{
    if (drawFunction == nullptr)
        return;

    if (!hasFlag(Velox::EntityFlags::Visible)) 
        return;

    drawFunction(*this);    
}

//
// Entity Node
//

bool Velox::EntityNode::isLeaf()
{
    return children.empty();
}

bool Velox::EntityNode::addNode(const Velox::EntityNode& node, const Velox::EntityNode& desiredParent)
{
    if (id == desiredParent.id)
    {
        children.push_back(node);
        return true;
    }

    for (EntityNode& child : children)
    {
        bool result = child.addNode(node, desiredParent);

        if (result)
            return true;
    }

    return false;
}

void Velox::EntityNode::update(double& deltaTime, Entity* parent, bool isRoot)
{
    Velox::Entity* currentEntity = nullptr;

    if (!isRoot)
    {
        currentEntity = s_entityManager.getMut(id);
        if (currentEntity == nullptr)
            return;

        currentEntity->update(deltaTime, parent);
    }

    for (Velox::EntityNode& child : children)
        child.update(deltaTime, currentEntity);
}

//
// EntityTreeView
//

bool Velox::EntityTreeView::addNode(const Velox::EntityHandle& handle, const Velox::EntityHandle& desiredParent)
{
    return root.addNode({ handle }, { desiredParent });
}

void Velox::EntityTreeView::updateEntities(double &deltaTime)
{
    root.update(deltaTime, nullptr, true);
}

//
// EntityManager 
//

Velox::EntityManager::EntityManager()
{
    for (uint32_t i = 0; i < MAX_ENTITIES; ++i)
    {
        freeIndices[i] = MAX_ENTITIES - i - 1;
        generations[i] = 1;
    }
}

Velox::EntityHandle Velox::EntityManager::createEntity(const Velox::EntityHandle& parent)
{
    assert(freeIndicesCount > 0 && "WARNING: Entity pool exhausted!");

    freeIndicesCount -= 1;
    uint32_t index = freeIndices[freeIndicesCount];
    entities[index] = { makeHandle(index) };
    entities[index].parent = parent;

    generateTreeView();

    return makeHandle(index);
}

Velox::Entity* Velox::EntityManager::getCreateEntity(const Velox::EntityHandle& parent)
{
    assert(freeIndicesCount > 0 && "WARNING: Entity pool exhausted!");

    freeIndicesCount -= 1;
    uint32_t index = freeIndices[freeIndicesCount];
    entities[index] = { makeHandle(index)};
    entities[index].parent = parent;

    generateTreeView();

    return &entities[index];
}

Velox::EntityHandle Velox::EntityManager::makeHandle(uint32_t index) const
{
    return Velox::EntityHandle { index, generations[index] };
}

Velox::Entity& Velox::EntityManager::get(Velox::EntityHandle handle)
{
    assert(isAlive(handle) && "WARNING: Non-alive entity queried!");

    return entities[handle.index];
}

Velox::Entity* Velox::EntityManager::getMut(Velox::EntityHandle handle)
{
    if (!isAlive(handle))
        return nullptr;

    return &entities[handle.index];
}

void Velox::EntityManager::destroyEntity(Velox::EntityHandle handle)
{
    if (!isAlive(handle)) return;

    generations[handle.index] += 1; // Invalidate stale handles;
    freeIndices[freeIndicesCount] = handle.index;
    freeIndicesCount += 1;

    generateTreeView();
}

void Velox::EntityManager::updateEntities(double& deltaTime)
{
    treeView.updateEntities(deltaTime);
}

void Velox::EntityManager::drawEntities()
{
    for (std::pair<EntityHandle, Entity*> pair : iter())
        pair.second->draw();
}

void Velox::EntityManager::generateTreeView()
{
    treeView.root = {};

    for (std::pair<EntityHandle, Entity*> pair : iter())
    {
        bool result = treeView.addNode(pair.first, pair.second->parent);
        if (!result)
            LOG_WARN("Failed to add entity to tree view");
    }
}

void Velox::EntityManager::getTreeViewHandlesAsVector(std::vector<EntityHandle>* handles)
{
    LOG_WARN("NOT IMPLEMENTED");   
}

void Velox::EntityManager::getTreeViewEntitiesAsVector(std::vector<Entity*>* handles)
{
    LOG_WARN("NOT IMPLEMENTED");   
}

bool Velox::EntityManager::isAlive(const Velox::EntityHandle& handle) const
{
    if (!handle.isValid())
        return false;

    return generations[handle.index] == handle.generation;
}

bool Velox::EntityManager::isIndexFree(uint32_t index) const
{
    for (size_t i = 0; i < freeIndicesCount; ++i) {
        if (freeIndices[i] == index)
            return true;
    }

    return false;
}

//
// Iterator
//

Velox::EntityManager::EntityIterable Velox::EntityManager::iter()
{
    return EntityIterable { this };
}

Velox::EntityManager::EntityIterable::Iterator::Iterator(Velox::EntityManager* m, uint32_t i)
    : manager(m), index(i)
{
    skipDead();
}

void Velox::EntityManager::EntityIterable::Iterator::skipDead()
{
    while (index < MAX_ENTITIES && manager->isIndexFree(index))
        index += 1;
}

bool Velox::EntityManager::EntityIterable::Iterator::operator!=(const Iterator& other) const
{
    return index != other.index;
}

void Velox::EntityManager::EntityIterable::Iterator::operator++()
{
    ++index;
    skipDead();
}

std::pair<Velox::EntityHandle, Velox::Entity*> Velox::EntityManager::EntityIterable::Iterator::operator*() const
{
    EntityHandle handle = manager->makeHandle(index);
    return std::pair<EntityHandle, Entity*> { handle, &manager->entities[index] };
}

Velox::EntityManager::EntityIterable::Iterator Velox::EntityManager::EntityIterable::begin()
{
    return Iterator { manager, 0 };
}

Velox::EntityManager::EntityIterable::Iterator Velox::EntityManager::EntityIterable::end()
{
    return Iterator { manager, MAX_ENTITIES };
}

