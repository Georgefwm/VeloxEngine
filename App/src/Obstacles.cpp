#include "Obstacles.h"
#include <PCH.h>
#include <SDL3/SDL_stdinc.h>

#include "PlaneGame.h"
#include "Asset.h"
#include "Entity.h"
#include "Rendering/Renderer.h"

static f64 s_spawnRateSecs = 4.0f;
static f64 s_spawnCooldown = 6.0f;

static f32 s_gapSize = 500.0f;
static f32 s_offsetMax;

void setupObstacles(f32 gapOffset)
{
    vec2 windowSize = Velox::getWindowSize();

    Velox::Entity* top = Velox::getEntityManager()->getCreateEntity();
    Velox::Entity* bot = Velox::getEntityManager()->getCreateEntity();

    top->type = EntityType::Spike;
    bot->type = EntityType::Spike;

    f32 spikeSize = (windowSize.y / 2.0f) + s_offsetMax - (s_gapSize / 2.0f) + 5.0f;
    f32 midPoint  = (windowSize.y / 2.0f) + gapOffset;

    top->scale = vec2(spikeSize / 8.0f, spikeSize);
    bot->scale = vec2(spikeSize / 8.0f, spikeSize);

    top->position = vec3(windowSize.x + 30.0f, midPoint + (s_gapSize / 2.0f) + 5.0f, 0.0f);
    top->rotation = 180.0f;  // upside down.

    bot->position = vec3(windowSize.x + 30.0f, midPoint - spikeSize - (s_gapSize / 2.0f)- 0.5, 0.0f);

    top->texture = Velox::getAssetManager()->loadTexture("rock_ice.png");
    bot->texture = Velox::getAssetManager()->getTexture("rock_ice.png");

    top->setFlag(Velox::EntityFlags::Visible,  true);
    bot->setFlag(Velox::EntityFlags::Visible,  true);

    top->updateFunction = updateObstacles;
    bot->updateFunction = updateObstacles;

    // Use child entities for colliders.
    Velox::Entity* topCollider = Velox::getEntityManager()->getCreateEntity(top->id);
    Velox::Entity* botCollider = Velox::getEntityManager()->getCreateEntity(bot->id);

    topCollider->position.x = -60.0f;
    botCollider->position.x = 120.0f;

    topCollider->scale.x = 0.2f;
    botCollider->scale.x = 0.2f;

    topCollider->setFlag(Velox::EntityFlags::Collides, true);
    botCollider->setFlag(Velox::EntityFlags::Collides, true);
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

    s_offsetMax = Velox::getWindowSize().y - s_gapSize - 100.0f;
}

void updateSpawner(Velox::Entity& e, const double& deltaTime)
{
    if (s_spawnCooldown < 0.0)
    {
        s_spawnCooldown = s_spawnRateSecs;

        float offset = (SDL_randf() - 0.5f) * s_offsetMax;;
        setupObstacles(offset);
    }

    s_spawnCooldown -= deltaTime;
}
