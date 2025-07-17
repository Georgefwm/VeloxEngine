#include "Plane.h"
#include <PCH.h>

#include "PlaneGame.h"

#include "Asset.h"
#include "Input.h"
#include "Rendering/Renderer.h"
#include <SDL3/SDL_scancode.h>
#include <glm/ext/scalar_constants.hpp>

constexpr f32 GRAVITY_FACTOR     = 10.0f;
constexpr f32 JUMP_IMPULSE_FORCE = 7.0f;

static f32  s_verticalVelocity;

void setupPlane()
{
    Velox::Entity* e = Velox::getEntityManager()->getCreateEntity();
    e->type = EntityType::Plane;
     
    e->position.x = 200;
    e->position.y = Velox::getWindowSize().y / 2.0f;

    e->texture = Velox::getAssetManager()->loadTexture("plane_red_1.png");
    e->scale = vec2(200.0f, 200.0f);

    e->setFlag(Velox::EntityFlags::Visible,  true);
    e->setFlag(Velox::EntityFlags::Collides, true);

    e->updateFunction = updatePlane;

    s_verticalVelocity = JUMP_IMPULSE_FORCE;
}

void updatePlane(Velox::Entity& e, const double& deltaTime)
{
    float windowHeight = Velox::getWindowSize().y;

    // Lose if plane falls out of bounds.
    if (e.position.y > windowHeight - (e.scale.y * 0.5))
    {
        changeGameStage(GameStage::PostRound);
        return;
    }

    // Block input if too high up (out of bounds).
    if (Velox::isKeyPressed(SDL_SCANCODE_SPACE) && e.position.y < windowHeight)
        s_verticalVelocity = JUMP_IMPULSE_FORCE;

    // Check for collisions.
    std::vector<Velox::EntityHandle> overlaps = e.getOverlappingEntities();

    // Currenly anything that would overlap would kill the plane.
    if (overlaps.size() > 0)
    {
        changeGameStage(GameStage::PostRound);
        return;
    }

    s_verticalVelocity -= GRAVITY_FACTOR * deltaTime;
    e.position.y -= s_verticalVelocity;

    // update orientation
    vec2 pointDirection = vec2(getGameState()->scrollSpeed, s_verticalVelocity);
    e.rotation = (glm::atan(pointDirection.y, pointDirection.x) / glm::pi<float>()) * 180.0;
}

