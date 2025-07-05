#include "Core.h"
#include "Asset.h"
#include "Debug.h"
#include "Event.h"
#include "Rendering/Renderer.h"
#include "Text.h"
#include "Timing.h"
#include "Types.h"
#include "Entity.h"

#include <imgui.h>
#include <iostream>
#include <ostream>

Velox::EntityManager* g_entityManager;

u32 g_fontShaderId;
Velox::Font* g_font;

Velox::EntityHandle e1;
Velox::EntityHandle e2;
float direction = 1.0;

void updateStar(Velox::Entity& e, const double& getDeltaTime)
{
    e.position.x += (300 * direction) * getDeltaTime;
    // e.rotation   += (40 * direction) * getDeltaTime;

    ivec2 windowSize = Velox::getWindowSize();
    if (e.position.x > windowSize.x * 0.95) direction = -1.0;
    if (e.position.x < windowSize.x * 0.05) direction =  1.0;
}

void drawStar(Velox::Entity& e)
{
    vec3 usePosition = e.absolutePosition;

    if (e.drawFromCenter)
    {
        usePosition.x -= e.absoluteScale.x * 0.5f;
        usePosition.y -= e.absoluteScale.y * 0.5f;
    }

    if (e.texture != nullptr)
        Velox::drawRotatedQuad(usePosition, e.absoluteScale, e.colorTint, e.absoluteRotation, e.texture);
    else
        Velox::drawRotatedQuad(usePosition, e.absoluteScale, e.colorTint, e.absoluteRotation, 0);
}

void doUpdates(double& deltaTime)
{
    g_entityManager->treeView.updateEntities(deltaTime);
}

void doTestStuff()
{
    Velox::drawQuad(vec3(100.0f, 100.0f, 0.0f), vec2(200.0f, 200.0f), vec4(1.0f));

    Velox::drawRect(vec3(90.0f, 90.0f, 0.0f), vec2(220.0f, 220.0f), vec4(1.0f));

    Velox::drawQuad(vec3(100.0f, 400.0f, 0.0f), vec2(200.0f, 200.0f), vec4(1.0f),
            Velox::getAssetManager()->getTexture("missing_texture.png"));

    Velox::drawQuad(vec3(400.0f, 100.0f, 0.0f), vec2(500.0f, 500.0f), vec4(1.0f), g_font->texture);

    Velox::drawQuad(vec3(950.0f, 100.0f, 0.0f), vec2(1200.0f, 500.0f), vec4(1.0f));

    Velox::TextDrawStyle textInfo {};
    textInfo.textSize = 130.0f;
    Velox::pushTextStyle(textInfo);

    Velox::TextDrawStyle editorStyle {};
    Velox::textStyleEditor(&editorStyle, true);

    Velox::pushTextStyle(editorStyle);

    Velox::pushFont("martius.ttf");
    
    Velox::drawText("<- MSDF Texture", vec3(1000.0f, 500.0f, 0.0f));
    Velox::popFont();
    Velox::drawText("text in another font", vec3(1000.0f, 400.0f, 0.0f));

    vec3 continueTextPosition = vec3(1000.0f, 300.0f, 0.0f);
    Velox::TextContinueInfo contInfo = Velox::drawText("con", continueTextPosition);
    contInfo = Velox::drawColoredText("tin", continueTextPosition, COLOR_RED, &contInfo);
    contInfo = Velox::drawColoredText("ued ", continueTextPosition, COLOR_GREEN, &contInfo);
    contInfo = Velox::drawColoredText("text and\non ", continueTextPosition, COLOR_BLUE, &contInfo);
    contInfo = Velox::drawColoredText("new line", continueTextPosition, COLOR_GRAY_LIGHT, &contInfo);

    vec3 testPos(2200.0f, 300.0f, 0.0f);
    const char* testText = "boundtest";
    Velox::drawText(testText, testPos);

    Velox::Rectangle bounds;
    Velox::getStringBounds(testText, &bounds);

    bounds.x += testPos.x;
    bounds.y += testPos.y;

    Velox::drawRect(bounds, COLOR_GREEN);

    Velox::popTextStyle();
    Velox::popTextStyle();

    Velox::drawLine(vec3(100.0f, 615.0f, 0.0f), vec3(900.0f, 615.0f, 0.0f), COLOR_RED);

    float startHeight = 620.0f;
    editorStyle.textSize = 20.0f;
    for (int i = 0; i < 10; i++)
    {
        startHeight += editorStyle.textSize;
        editorStyle.textSize = editorStyle.textSize * 1.5f;
        Velox::pushTextStyle(editorStyle);
        Velox::drawText("SAMPLE TEXT", vec3(100.0f, startHeight, 0.0f));
        Velox::popTextStyle();
    }
}

void doRenderingStuff()
{
    // doTestStuff();
    g_entityManager->drawEntities();
    ImGui::ShowDemoWindow();
}

void run()
{
    Velox::init();

    g_entityManager = Velox::getEntityManager();
    e1 = g_entityManager->createEntity();
    Velox::Entity* e = g_entityManager->getMut(e1);
    
    e->position = vec3(100.0f, 540.0f, 0.0f);
    e->scale    = vec2(100.0f, 100.0f);
    e->collider = { 0.0f, 0.0f, e->scale.x, e->scale.y };
    e->texture  = Velox::getAssetManager()->loadTexture("star.png");
    e->setFlag(Velox::EntityFlags::Visible, true);
    e->setFlag(Velox::EntityFlags::Collides, true);
    // Set update/draw functions like this.
    e->updateFunction = updateStar;
    e->drawFunction = drawStar;

    // Set e1 as parent of p2.
    e2 = g_entityManager->createEntity(e1);
    e = g_entityManager->getMut(e2);

    e->position = vec3(30.0f, 0.0f, 0.0f);
    e->scale    = vec2(1.0f);
    e->texture  = Velox::getAssetManager()->getTexture("star.png");
    e->setFlag(Velox::EntityFlags::Visible, true);
    e->drawFunction = drawStar;
    // No update function set so this entity will not update.

    g_font = Velox::getAssetManager()->loadFont("martius.ttf");

    g_fontShaderId = 0;

    while (!Velox::quitRequested())
    {
        Velox::getEventPublisher()->processEvents();

        Velox::updateGame(doUpdates);

        doRenderingStuff();

        Velox::submitFrameData();

        // Calculate after frame present.
        Velox::calculateDeltaTime();
    }

    // Cleanup.
    Velox::deInit();
}

int main(int argc, char** argv)
{   
    try
    {
        run();
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return 0;
}
