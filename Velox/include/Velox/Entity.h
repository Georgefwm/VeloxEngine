#pragma once

#include "Velox.h"

#include <utility>  // pair

constexpr size_t MAX_ENTITIES = 1024;

namespace Velox {

struct Texture;
struct EntityManager;

enum EntityFlags : uint32_t {
    None     = 0,
    Updates  = 1 << 0,
    Visible  = 1 << 1,
    Static   = 1 << 2,
    Collides = 1 << 3
};

struct VELOX_API EntityHandle {
    uint32_t index = 0;
    uint32_t generation = 0;

    bool isValid() const { return generation != 0; }

    bool operator==(const EntityHandle& other) const
    {
        if (index      != other.index)      return false;
        if (generation != other.generation) return false;

        return true;
    }
    bool operator!=(const EntityHandle& other) const { return !(*this == other); }
};

void initEntitySystem();
VELOX_API Velox::EntityManager* getEntityManager();

struct VELOX_API Entity {
    // Core
    EntityHandle id;
    u32 flags = Velox::EntityFlags::Updates;

    // Functions
    std::function<void(Velox::Entity&, double&)> updateFunction = nullptr;
    std::function<void(Velox::Entity&)> drawFunction = nullptr;

    EntityHandle parent = {};

    // Transform
    // Usually you want to write these.
    vec3  position = vec3(0.0f);
    float rotation = 0;
    vec2  scale    = vec4(10.0f);

    // Usually you only want to read these.
    vec3  absolutePosition = vec3(0.0f);
    float absoluteRotation = 0;
    vec2  absoluteScale    = vec4(10.0f);

    // Collision
    bool collideFromCenter = false;
    Velox::Rectangle collider = { 0.0f, 0.0f, scale.x, scale.y };

    // Rendering
    Velox::Texture* texture = nullptr;
    bool drawFromCenter = false;
    vec4 colorTint = vec4(1.0f); // colors aren't clamped so can use for flashe effects for example.

    bool hasFlag(EntityFlags flag) const
    {   
        return (flags & flag) != 0;
    }

    void setFlag(EntityFlags flag, int state)
    {
        if (state) flags |=  flag;
        else       flags &= ~flag;
    }

    // Quality of life/reminder that entity tree view needs to be updated to act on
    // parent/child relationships. See comment on EntityManager::generateTreeView().
    void setParent(const EntityHandle& handle);

    // Only for calling update/draw function members.
    void update(double& getDeltaTime);
    // Speeds up hierarchical iteration slightly (probably).
    void update(double& getDeltaTime, Entity* parentRef);
    void draw();
};

struct VELOX_API EntityNode {
    Velox::EntityHandle id {};
    std::vector<EntityNode> children;

    bool isLeaf();
    bool addNode(const Velox::EntityNode& node, const Velox::EntityNode& desiredParent);
    void update(double& deltaTime, Velox::Entity* parent = nullptr, bool isRoot = false);
    void destroyChildren(const EntityHandle& handle, bool isRoot = false);
};

struct VELOX_API EntityTreeView {
    EntityNode root {};

    bool addNode(const EntityHandle& node, const EntityHandle& desiredParent = {});
    // void removeNode(EntityHandle& handle);
    void updateEntities(double& deltaTime);
};

// Ideas:
// Could store entites in arrays that are sorted is dfs or bfs ordering for fast tree traversal. 
//  - e.g. Insert child entities after parents(in memory) for dfs, not sure what this is called. Flat tree?
//  - Would have to shuffle indices every insertion/deletion, probably quite fast for less dynamic scenes.
// Multiple arrays for different ordering, i.e. draw order, tree order, etc.
struct VELOX_API EntityManager {
    // GM: Will probably need to use custom allocator for this when entity data gets big enough.
    Entity entities[MAX_ENTITIES];
    uint32_t generations[MAX_ENTITIES];
    uint32_t freeIndices[MAX_ENTITIES];
    size_t freeIndicesCount = MAX_ENTITIES;
    Velox::EntityTreeView treeView;

    EntityManager();

    EntityHandle makeHandle(uint32_t index) const;

    EntityHandle createEntity(const EntityHandle& parent = {});
    Entity* getCreateEntity(const EntityHandle& parent = {});

    Entity& get(EntityHandle handle);
    Entity* getMut(EntityHandle handle);

    void destroyEntity(const EntityHandle& handle);
    void destroyEntityInternal(const EntityHandle& handle);

    void updateEntities(double& deltaTime);
    void drawEntities();

    // Currently this needs to be called after adding a parent to an entity when added in any way 
    // other than on first creation with the optional parent parameter on createEntity().
    // This is pretty dusty, but hierachy is suprisingly tricky to get right (without blowing up
    // code complextity). addParent() utility method should be used as a reminder.
    void generateTreeView();

    void getTreeViewHandlesAsVector(std::vector<EntityHandle>* handles);
    void getTreeViewEntitiesAsVector(std::vector<Entity*>* handles);

    bool isAlive(const EntityHandle& handle) const;

    bool isIndexFree(uint32_t index) const;

    struct EntityIterable;
    EntityIterable iter();
};

struct VELOX_API EntityManager::EntityIterable {
    struct VELOX_API Iterator {
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

