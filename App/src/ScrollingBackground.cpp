#include "ScrollingBackground.h"
#include <PCH.h>

#include "PlaneGame.h"
#include "Asset.h"
#include "Entity.h"

static Velox::EntityHandle s_e1;
static Velox::EntityHandle s_e2;

Velox::EntityHandle createBackgroundEntity(float initialXPosition)
{
    Velox::Entity* e = Velox::getEntityManager()->getCreateEntity();
     
    e->position.x = initialXPosition;

    e->texture = Velox::getAssetManager()->loadTexture("background.png");
    e->scale = Velox::getWindowSize();

    // Add a small amount of overlap to avoid pixel gaps between backgrounds.
    e->scale.x += 1.0f;

    e->setFlag(Velox::EntityFlags::Visible,  true);
    e->setFlag(Velox::EntityFlags::Visible,  true);

    e->updateFunction = updateScrollingBackground;

    return e->id;
}

void setupScrollingBackground()
{
    s_e1 = createBackgroundEntity(0.0f);
    s_e2 = createBackgroundEntity(Velox::getWindowSize().x);
}

void updateScrollingBackground(Velox::Entity& e, const double& deltaTime)
{
    // Substract 1 as it is only intended to be overlap, not actual size for calculations.
    // See comment in setup.
    if (e.position.x + e.scale.x - 1.0f <= 0.0f)
        e.position.x += (e.scale.x - 1.0f) * 2.0f;

    e.position.x -= (getGameState()->scrollSpeed / 2.0f) * deltaTime;
}
