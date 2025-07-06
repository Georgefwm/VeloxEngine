#include "Obstacles.h"
#include <PCH.h>

#include "PlaneGame.h"
#include "Asset.h"
#include "Entity.h"
#include "Rendering/Renderer.h"

static f64 s_spawnRateSecs = 4.0f;
static f64 s_spawnCooldown = 6.0f;

static f32 s_spikeGap = 500.0f;

void setupObstacles()
{
    vec2 windowSize = Velox::getWindowSize();

    Velox::Entity* top = Velox::getEntityManager()->getCreateEntity();
    Velox::Entity* bot = Velox::getEntityManager()->getCreateEntity();

    top->type = EntityType::Spike;
    bot->type = EntityType::Spike;

    top->scale = vec2(100.0f, 200.0f);
    bot->scale = vec2(100.0f, 200.0f);

    top->position.x = windowSize.x + 30.0f;
    top->position.y = windowSize.y - top->scale.y;
    top->rotation = 180.0f;  // upside down.

    bot->position.x = windowSize.x + 30.0f;
    bot->position.y = 0.0f;


    top->texture = Velox::getAssetManager()->loadTexture("rock_ice.png");
    bot->texture = Velox::getAssetManager()->loadTexture("rock_ice.png");

    top->setFlag(Velox::EntityFlags::Visible,  true);
    top->setFlag(Velox::EntityFlags::Collides, true);
    bot->setFlag(Velox::EntityFlags::Visible,  true);
    bot->setFlag(Velox::EntityFlags::Collides, true);

    top->updateFunction = updateObstacles;
    bot->updateFunction = updateObstacles;
}

void updateObstacles(Velox::Entity& e, const double& deltaTime)
{
    if (e.position.x + e.scale.x < 0)
    {
        Velox::getEntityManager()->destroyEntity(e.id);
        return;
    }

    e.position.x -= getGameState()->scrollSpeed * deltaTime;
}

void setupSpawner()
{
    Velox::Entity* spawner = Velox::getEntityManager()->getCreateEntity();
    spawner->updateFunction = updateSpawner;

    s_spawnRateSecs = 3.0f;
    s_spawnCooldown = s_spawnRateSecs;
}

void updateSpawner(Velox::Entity& e, const double& deltaTime)
{
    if (s_spawnCooldown < 0.0)
    {
        s_spawnCooldown = s_spawnRateSecs;
        setupObstacles();
    }

    s_spawnCooldown -= deltaTime;
}
