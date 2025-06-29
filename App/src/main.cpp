#include "Asset.h"
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

Velox::EntityManager g_entityManager;

u32 g_fontShaderId;
Velox::Font* g_font;

Velox::EntityHandle e1;
float direction = 1.0;

void HandleEvent(Velox::Event* event)
{

}

void DoUpdates(double& deltaTime)
{
    Velox::Entity* e = g_entityManager.getMut(e1);

    e->position.x += (300 * direction) * deltaTime;
    e->rotation += (40 * direction) * deltaTime;

    ivec2 windowSize = Velox::GetWindowSize();
    if (e->position.x > windowSize.x * 0.95) direction = -1.0;
    if (e->position.x < windowSize.x * 0.05) direction =  1.0;
}

void DoRenderingStuff()
{
    Velox::DrawQuad(vec3(100.0f, 100.0f, 0.0f), vec2(200.0f, 200.0f), vec4(1.0f));

    Velox::DrawRect(vec3(90.0f, 90.0f, 0.0f), vec2(220.0f, 220.0f), vec4(1.0f));

    Velox::DrawQuad(vec3(100.0f, 400.0f, 0.0f), vec2(200.0f, 200.0f), vec4(1.0f),
            Velox::GetAssetManager()->GetTexture("missing_texture.png"));

    Velox::DrawQuad(vec3(400.0f, 100.0f, 0.0f), vec2(500.0f, 500.0f), vec4(1.0f), g_font->texture);

    Velox::Entity* e = g_entityManager.getMut(e1);
    e->Draw(true);

    Velox::DrawQuad(vec3(950.0f, 100.0f, 0.0f), vec2(1200.0f, 500.0f), vec4(1.0f));

    Velox::TextDrawStyle textInfo {};
    textInfo.textSize = 130;
    Velox::PushTextStyle(textInfo);

    Velox::PushFont("martius.ttf");
    Velox::DrawText("<- MSDF Texture", vec3(1000.0f, 500.0f, 0.0f));
    Velox::PopFont();
    Velox::DrawText("text in another font", vec3(1000.0f, 400.0f, 0.0f));

    vec3 continueTextPosition = vec3(1000.0f, 300, 0.0f);
    Velox::TextContinueInfo contInfo = Velox::DrawText("con", continueTextPosition);
    contInfo = Velox::DrawColoredText("tin", continueTextPosition, COLOR_RED, &contInfo);
    contInfo = Velox::DrawColoredText("ued ", continueTextPosition, COLOR_GREEN, &contInfo);
    contInfo = Velox::DrawColoredText("text and\non ", continueTextPosition, COLOR_BLUE, &contInfo);
    contInfo = Velox::DrawColoredText("new line", continueTextPosition, COLOR_GRAY_LIGHT, &contInfo);

    Velox::PopTextStyle();

    Velox::DrawLine(vec3(100.0f, 615.0f, 0.0f), vec3(900.0f, 615.0f, 0.0f), COLOR_RED);


    float startHeight = 620.0f;
    textInfo.textSize = 20.0f;
    for (int i = 0; i < 10; i++)
    {
        startHeight += textInfo.textSize;
        textInfo.textSize = textInfo.textSize * 1.5f;
        Velox::PushTextStyle(textInfo);
        Velox::DrawText("SAMPLE TEXT", vec3(100.0f, startHeight, 0.0f));
        Velox::PopTextStyle();
    }

    ImGui::ShowDemoWindow();
}

void run()
{
    Velox::Init();

    g_entityManager = Velox::EntityManager();
    e1 = g_entityManager.createEntity();
    Velox::Entity* e = g_entityManager.getMut(e1);
    
    e->position = vec3(100, 300, 0);
    e->size = vec2(100, 100);
    e->texture = Velox::GetAssetManager()->LoadTexture("star.png");
    e->flags |= Velox::EntityFlags::Visible;

    g_font = Velox::GetAssetManager()->LoadFont("martius.ttf");

    if (g_font == nullptr)
        printf("font is null\n");

    g_fontShaderId = 0;

    while (!Velox::QuitRequested())
    {

        Velox::Event event;
        while (Velox::PollEvents(&event))
        {
            HandleEvent(&event);
            // Do something with events.
        }

        Velox::UpdateGame(DoUpdates);

        Velox::StartFrame();
        
        DoRenderingStuff();

        Velox::EndFrame();

        // Calculate after frame present.
        Velox::CalculateDeltaTime();
    }

    // Cleanup.
    Velox::DeInit();
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
