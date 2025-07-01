#include "Asset.h"
#include "Debug.h"
#include "Event.h"

#include "Rendering/Renderer.h"
#include "Text.h"
#include "Timing.h"
#include "Types.h"
#include "Velox.h"
#include "Entity.h"

#include <imgui.h>
#include <iostream>
#include <ostream>

Velox::EntityManager* g_entityManager;

u32 g_fontShaderId;
Velox::Font* g_font;

Velox::EntityHandle e1;
float direction = 1.0;

void updateStar(Velox::Entity& e, double getDeltaTime)
{
    e.position.x += (300 * direction) * getDeltaTime;
    e.rotation += (40 * direction) * getDeltaTime;

    ivec2 windowSize = Velox::getWindowSize();
    if (e.position.x > windowSize.x * 0.95) direction = -1.0;
    if (e.position.x < windowSize.x * 0.05) direction =  1.0;
}

void drawStar(Velox::Entity& e)
{
    vec3 usePosition = e.position;

    if (e.drawFromCenter)
    {
        usePosition.x -= e.size.x * 0.5f;
        usePosition.y -= e.size.y * 0.5f;
    }

    if (e.texture != nullptr)
        Velox::drawRotatedQuad(usePosition, e.size, e.colorOverride, e.rotation, e.texture);
    else
        Velox::drawRotatedQuad(usePosition, e.size, e.colorOverride, e.rotation, 0);
}

void handleEvent(Velox::Event* event)
{

}

void doUpdates(double& getDeltaTime)
{
    for (auto e : g_entityManager->iter())
    {
        Velox::EntityHandle handle = e.first;
        Velox::Entity* entity = e.second;

        entity->update(getDeltaTime);
    }

    Velox::Entity* e = g_entityManager->getMut(e1);
}

void doRenderingStuff()
{
    Velox::drawQuad(vec3(100.0f, 100.0f, 0.0f), vec2(200.0f, 200.0f), vec4(1.0f));

    Velox::drawRect(vec3(90.0f, 90.0f, 0.0f), vec2(220.0f, 220.0f), vec4(1.0f));

    Velox::drawQuad(vec3(100.0f, 400.0f, 0.0f), vec2(200.0f, 200.0f), vec4(1.0f),
            Velox::getAssetManager()->getTexture("missing_texture.png"));

    Velox::drawQuad(vec3(400.0f, 100.0f, 0.0f), vec2(500.0f, 500.0f), vec4(1.0f), g_font->texture);

    Velox::Entity* e = g_entityManager->getMut(e1);
    e->draw();

    Velox::drawQuad(vec3(950.0f, 100.0f, 0.0f), vec2(1200.0f, 500.0f), vec4(1.0f));

    Velox::TextDrawStyle textInfo {};
    textInfo.textSize = 130;
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

    ImGui::ShowDemoWindow();
}

void run()
{
    Velox::init();

    g_entityManager = Velox::GetEntityManager();
    e1 = g_entityManager->createEntity();
    Velox::Entity* e = g_entityManager->getMut(e1);
    
    e->position = vec3(100, 300, 0);
    e->size = vec2(100, 100);
    e->texture = Velox::getAssetManager()->loadTexture("star.png");
    e->flags |= Velox::EntityFlags::Visible;

    // Set update/draw functions like this.
    e->updateFunction = updateStar;
    e->drawFunction = drawStar;

    g_font = Velox::getAssetManager()->loadFont("martius.ttf");

    g_fontShaderId = 0;

    while (!Velox::quitRequested())
    {

        Velox::Event event;
        while (Velox::pollEvents(&event))
        {
            handleEvent(&event);
            // Do something with events.
        }

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
