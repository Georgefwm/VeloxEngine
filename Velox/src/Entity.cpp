#include "Entity.h"
#include "Rendering/Renderer.h"
#include "Util.h"
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

void Velox::Entity::setParent(const EntityHandle& handle)
{
    if (!handle.isValid())
        return;

    parent = handle;

    // Only reason this function exists.
    // s_entityManager.generateTreeView();
}

void Velox::Entity::update(const double& deltaTime)
{
    // Could maybe add flag for enabling/disabling update. Dunno yet.

    if (updateFunction == nullptr)
        return;
    
    updateFunction(*this, deltaTime);
}

void Velox::Entity::update(const double& deltaTime, Velox::Entity* parentRef)
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

    collider.x = absolutePosition.x;
    collider.y = absolutePosition.y;
    collider.w = absoluteScale.x;
    collider.h = absoluteScale.y;

    if (collideFromCenter)
    {
        collider.x -= scale.x / 2.0f;
        collider.y -= scale.y / 2.0f;
    }

    if (!hasFlag(Velox::EntityFlags::Updates))
        return;

    if (updateFunction == nullptr)
        return;
    
    updateFunction(*this, deltaTime);
}

void defaultDrawSprite(Velox::Entity& e)
{
    vec3 usePosition = e.absolutePosition;

    if (e.drawFromCenter)
    {
        usePosition.x -= e.absoluteScale.x * 0.5f;
        usePosition.y -= e.absoluteScale.y * 0.5f;
    }

    Velox::drawRotatedQuad(usePosition, e.absoluteScale, e.colorTint, e.absoluteRotation, e.texture);
}

void Velox::Entity::draw()
{
    if (!hasFlag(Velox::EntityFlags::Visible)) 
        return;

    if (drawFunction == nullptr)
    {
        // Just use this as a safe default.
        defaultDrawSprite(*this);
        return;
    }

    drawFunction(*this);    
}

std::vector<Velox::EntityHandle> Velox::Entity::getOverlappingEntities()
{
    std::vector<Velox::EntityHandle> overlaps;

    for (auto entityPair : s_entityManager.iter())
    {
        if (id == entityPair.first)
            continue;

        if (!entityPair.second->hasFlag(Velox::EntityFlags::Collides))
            continue;

        if (!Velox::isOverlapping(collider, entityPair.second->collider))
            continue;
        
        overlaps.push_back(entityPair.second->id);
    }

    return overlaps;
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

void Velox::EntityNode::update(const double& deltaTime, Entity* parent, bool isRoot)
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

void Velox::EntityNode::destroyChildren(const Velox::EntityHandle& handle, bool isRoot)
{
    // Not a node that needs touching, just propogate request.
    if (handle != id)
    {
        for (Velox::EntityNode& child : children)
            child.destroyChildren(handle);

        return;
    }

    for (Velox::EntityNode& child : children)
    {
        // Pass reference to own id to signal it should be removed.
        child.destroyChildren(child.id);
    }

    // Mark for death, wait until end of frame to remove.
    s_entityManager.getMut(id)->setFlag(Velox::EntityFlags::Dead, true);
}

//
// EntityTreeView
//

bool Velox::EntityTreeView::addNode(const Velox::EntityHandle& handle, const Velox::EntityHandle& desiredParent)
{
    return root.addNode({ handle }, { desiredParent });
}

void Velox::EntityTreeView::updateEntities(const double& deltaTime)
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

// TODO: Allow child entities to be created before thier parent. 
Velox::EntityHandle Velox::EntityManager::createEntity(const Velox::EntityHandle& parent)
{
    if (freeIndicesCount <= 0)
    {
        LOG_WARN("Entity pool exhausted");
        return {};
    }

    freeIndicesCount -= 1;
    uint32_t index = freeIndices[freeIndicesCount];
    entities[index] = { makeHandle(index) };
    entities[index].parent = parent;

    isTreeDirty = true;

    return entities[index].id;
}

Velox::Entity* Velox::EntityManager::getCreateEntity(const Velox::EntityHandle& parent)
{
    if (freeIndicesCount <= 0)
    {
        LOG_WARN("Entity pool exhausted");
        return {};
    }

    freeIndicesCount -= 1;
    uint32_t index = freeIndices[freeIndicesCount];
    entities[index] = { makeHandle(index)};
    entities[index].parent = parent;

    isTreeDirty = true;

    return &entities[index];
}

Velox::EntityHandle Velox::EntityManager::makeHandle(uint32_t index) const
{
    return Velox::EntityHandle { index, generations[index] };
}

Velox::Entity& Velox::EntityManager::get(Velox::EntityHandle handle)
{
    if (!isAlive(handle))
        LOG_WARN("Non-alive entity queried!");

    return entities[handle.index];
}

Velox::Entity* Velox::EntityManager::getMut(Velox::EntityHandle handle)
{
    if (!isAlive(handle))
        return nullptr;

    return &entities[handle.index];
}

void Velox::EntityManager::destroyEntity(const Velox::EntityHandle& handle)
{
    if (!isAlive(handle))
        return;

    treeView.root.destroyChildren(handle);

    isTreeDirty = true;
}

void Velox::EntityManager::destroyEntityInternal(const Velox::EntityHandle& handle)
{
    if (!isAlive(handle))
        return;

    generations[handle.index] += 1; // Invalidate stale handles;
    freeIndices[freeIndicesCount] = handle.index;
    freeIndicesCount += 1;
}

void Velox::EntityManager::destroyAllEntities()
{
    for (u32 i = 0; i < MAX_ENTITIES; i++)
    {
        entities[i]      = {};
        freeIndices[i]   = MAX_ENTITIES - i - 1;
        generations[i]  += 1;
        freeIndicesCount = MAX_ENTITIES;
    }

    isTreeDirty = true;
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

void Velox::EntityManager::postFrameUpdates()
{
    for (i32 i = MAX_ENTITIES - 1; i >= 0; i--)
    {
        if (entities[i].hasFlag(Velox::EntityFlags::Dead))
        {
            destroyEntityInternal(entities[i].id);
            isTreeDirty = true;
        }
    }

    generateTreeView();
}

void Velox::EntityManager::generateTreeView(bool forceUpdate)
{
    if (!isTreeDirty && !forceUpdate)
        return;

    treeView.root = {};

    for (std::pair<EntityHandle, Entity*> pair : iter())
    {
        bool result = treeView.addNode(pair.first, pair.second->parent);
        if (!result)
            LOG_WARN("Failed to add entity to tree view, parent '{}' not found", pair.second->parent);
    }

    isTreeDirty = false;
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

