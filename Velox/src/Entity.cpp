#include "Entity.h"
#include <PCH.h>

#include "Rendering/Renderer.h"

static Velox::EntityManager s_entityManager;

void Velox::InitEntitySystem()
{
    s_entityManager = Velox::EntityManager();
}

Velox::EntityManager* Velox::GetEntityManager()
{
    return &s_entityManager;
}

void Velox::Entity::update(double getDeltaTime)
{
    if (updateFunction == nullptr)
        return;
    
    updateFunction(*this, getDeltaTime);
}

void Velox::Entity::draw()
{
    if (drawFunction == nullptr)
        return;

    if (!(flags & Visible)) 
        return;

    drawFunction(*this);    
}

//
// EntityManager 
//

Velox::EntityManager::EntityManager() {
    for (uint32_t i = 0; i < MAX_ENTITIES; ++i) {
        freeIndices[i] = MAX_ENTITIES - i - 1;
    }
}

Velox::EntityHandle Velox::EntityManager::createEntity() {
    assert(freeIndicesCount > 0 && "WARNING: Entity pool exhausted!");

    freeIndicesCount -= 1;
    uint32_t index = freeIndices[freeIndicesCount];
    entities[index] = {};

    return this->makeHandle(index);
}

Velox::EntityHandle Velox::EntityManager::makeHandle(uint32_t index) const {
    return Velox::EntityHandle{ index, generations[index] };
}

Velox::Entity& Velox::EntityManager::get(Velox::EntityHandle handle) {
    assert(isAlive(handle) && "WARNING: Non-alive entity queried!");

    return entities[handle.index];
}

Velox::Entity* Velox::EntityManager::getMut(Velox::EntityHandle handle) {
    if (!isAlive(handle))
        return nullptr;

    return &entities[handle.index];
}

void Velox::EntityManager::destroyEntity(Velox::EntityHandle handle) {
    if (!isAlive(handle)) return;

    generations[handle.index] += 1; // Invalidate stale handles;
    freeIndices[freeIndicesCount] = handle.index;
    freeIndicesCount += 1;
}

bool Velox::EntityManager::isAlive(Velox::EntityHandle handle) const {
    return generations[handle.index] == handle.generation;
}

bool Velox::EntityManager::isIndexFree(uint32_t index) const {
    for (size_t i = 0; i < freeIndicesCount; ++i) {
        if (freeIndices[i] == index)
            return true;
    }

    return false;
}

//
// Iterator
//
Velox::EntityManager::EntityIterable Velox::EntityManager::iter() {
    return EntityIterable { this };
}

Velox::EntityManager::EntityIterable::Iterator::Iterator(Velox::EntityManager* m, uint32_t i)
    : manager(m), index(i) {
    skipDead();
}

void Velox::EntityManager::EntityIterable::Iterator::skipDead() {
    while (index < MAX_ENTITIES && manager->isIndexFree(index)) {
        index += 1;
    }
}

bool Velox::EntityManager::EntityIterable::Iterator::operator!=(const Iterator& other) const {
    return index != other.index;
}

void Velox::EntityManager::EntityIterable::Iterator::operator++() {
    ++index;
    skipDead();
}

std::pair<Velox::EntityHandle, Velox::Entity*> Velox::EntityManager::EntityIterable::Iterator::operator*() const {
    EntityHandle handle = manager->makeHandle(index);
    return std::pair<EntityHandle, Entity*> { handle, &manager->entities[index] };
}

Velox::EntityManager::EntityIterable::Iterator Velox::EntityManager::EntityIterable::begin() {
    return Iterator { manager, 0 };
}

Velox::EntityManager::EntityIterable::Iterator Velox::EntityManager::EntityIterable::end() {
    return Iterator { manager, MAX_ENTITIES };
}
