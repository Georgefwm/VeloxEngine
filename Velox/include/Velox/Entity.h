#pragma once

#include <cstdint>
#include <utility>


constexpr size_t MAX_ENTITIES = 1024;

namespace Velox {

struct EntityHandle {
    uint32_t index;
    uint32_t generation;

    bool operator==(const EntityHandle& other) const
    {
        return index == other.index && generation == other.generation;
    }
};

// GM: Not entirely sure how to expose this to the user without adding bunch of complexity.
// The general idea is that I want to have this be sub-classed to only one level deep so
// that we can take advantage of dynamic dispatch without having silly big inheritance trees.
struct Entity {
    int test = 0;
};

struct EntityManager {
    // GM: Will probably need to use custom allocator for this when entity data gets big enough.
    Entity entities[MAX_ENTITIES];
    uint32_t generations[MAX_ENTITIES];
    uint32_t freeIndices[MAX_ENTITIES];
    size_t freeIndicesCount = MAX_ENTITIES;

    EntityManager();

    EntityHandle makeHandle(uint32_t index) const;

    EntityHandle createEntity();

    Entity& get(EntityHandle handle);

    Entity* getMut(Velox::EntityHandle handle);

    void destroyEntity(EntityHandle handle);

    bool isAlive(EntityHandle handle) const;

    bool isIndexFree(uint32_t index) const;

    struct EntityIterable;
    EntityIterable iter();
};

struct EntityManager::EntityIterable {
    struct Iterator {
        EntityManager* manager;
        uint32_t index;

        Iterator(EntityManager* m, uint32_t i);

        void skipDead();

        bool operator!=(const Iterator& other) const;

        void operator++();

        std::pair<EntityHandle, Entity*> operator*() const;
    };

    EntityManager* manager;

    Iterator begin();
    Iterator end();
};

}

