#include "Plane.h"
#include "PlaneGame.h"
#include <PCH.h>

#include "Asset.h"
#include "Input.h"
#include "Rendering/Renderer.h"
#include "Util.h"
#include <SDL3/SDL_scancode.h>
#include <glm/ext/scalar_constants.hpp>

constexpr f32 GRAVITY_FACTOR     = 2.0f;
constexpr f32 JUMP_IMPULSE_FORCE = 10.0f;

static f32  s_verticalVelocity = 0.0f;

Velox::EntityHandle setupPlane()
{
    Velox::Entity* e = Velox::getEntityManager()->getCreateEntity();
     
    e->position.x = 200;
    e->position.y = Velox::getWindowSize().y / 2.0f;

    e->texture = Velox::getAssetManager()->loadTexture("plane_red_1");
    e->scale = vec2(200.0f, 200.0f);

    e->setFlag(Velox::EntityFlags::Visible,  true);
    e->setFlag(Velox::EntityFlags::Collides, true);

    e->updateFunction = updatePlane;

    return e->id;
}

void updatePlane(Velox::Entity& e, const double& deltaTime)
{
    return;

    if (Velox::isKeyPressed(SDL_SCANCODE_SPACE))
        s_verticalVelocity += JUMP_IMPULSE_FORCE;

    // Check for collisions.
    for (auto entityPair : Velox::getEntityManager()->iter())
    {
        if (e.id == entityPair.first)
            continue;

        if (!entityPair.second->hasFlag(Velox::EntityFlags::Collides))
            continue;

        if (Velox::isOverlapping(e.collider, entityPair.second->collider))
        {
            // die
        }
        
        s_verticalVelocity -= GRAVITY_FACTOR;
        e.position.y += s_verticalVelocity;

        // update orientation
        vec2 pointDirection = vec2(getGameState()->scrollSpeed, s_verticalVelocity);
        e.rotation = (glm::atan(pointDirection.y, pointDirection.x) / glm::pi<float>()) * 180.0;
    }
}

