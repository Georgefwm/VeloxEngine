#pragma once

#include "Core.h"

#include <cstdint>
#include <utility>  // pair

constexpr size_t MAX_ENTITIES = 1024;

namespace Velox {

enum EntityFlags : uint32_t {
    None    = 0,
    Visible = 1 << 0,
    Static  = 1 << 1,
};

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
//
// Maybe for now we just use a 'megastruct' where all data is just contained in the Entity struct.
// We can figure out extenting this in the API later.
struct Entity {
    // Core
    uint32_t flags = None;

    // Spacial
    vec3 position = vec3(0.0, 0.0, 0.0);
    float rotation = 0;

    // Rendering
    vec2 size = vec4(10.0);
    int textureIndex = 0;
    vec4 colorOverride = vec4(1.0);

    bool HasFlag(EntityFlags flag) const { return (flags & flag) != 0; }
    void Draw(bool centerOrigin = false);
};

// Ideas:
// Could store entites in arrays that are sorted is dfs or bfs ordering for fast tree traversal. 
//  - e.g. Insert child entities after parents(in memory) for dfs, not sure what this is called. Flat tree?
// Multiple arrays for different ordering, i.e. draw order, tree order, etc.
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

