#include "Asset.h"
#include "Event.h"

#include "Rendering/Renderer.h"
#include "Text.h"
#include "Velox.h"
#include "Entity.h"
#include "Util.h"

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

void UpdateGame(float deltaTime)
{
    Velox::Entity* e = g_entityManager.getMut(e1);

    e->position.x += (300 * direction) * deltaTime;
    // e->rotation += (40 * direction) * deltaTime;

    ivec2 windowSize = Velox::GetWindowSize();
    if (e->position.x > windowSize.x * 0.95) direction = -1.0;
    if (e->position.x < windowSize.x * 0.05) direction =  1.0;
}

void DoRenderingStuff()
{
    Velox::DrawQuad(vec3(100.0f, 100.0f, 0.0f), vec2(200.0f, 200.0f), vec4(1.0f));

    Velox::DrawRect(vec3(90.0f, 90.0f, 0.0f), vec2(220.0f, 220.0f), vec4(1.0f));

    Velox::DrawQuad(vec3(100.0f, 400.0f, 0.0f), vec2(200.0f, 200.0f), vec4(1.0f),
            Velox::GetAssetManager()->GetTextureID("missing_texture.png"));

    Velox::DrawQuad(vec3(400.0f, 100.0f, 0.0f), vec2(500.0f, 500.0f), vec4(1.0f), g_font->textureId);

    Velox::Entity* e = g_entityManager.getMut(e1);
    e->Draw(true);

    Velox::PushFont("martius.ttf");

    Velox::TextDrawInfo textInfo {};
    textInfo.position = vec3(1000.0f, 500.0f, 0.0f);
    textInfo.textSize = 90.0f;
    textInfo.color = vec4(1.0f);
    Velox::DrawText("<- MSDF Texture", textInfo);

    Velox::PopFont();

    textInfo.position.y -= 100.0f;
    Velox::DrawText("text in another font", textInfo);

    Velox::DrawLine(vec3(100.0f, 600.0f, 0.0f), vec3(900.0f, 600.0f, 0.0f), COLOR_RED);

    Velox::DrawLine(vec3(100.0f, 615.0f, 0.0f), vec3(900.0f, 615.0f, 0.0f), COLOR_RED);

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
    e->textureId = Velox::GetAssetManager()->LoadTexture("star.png");
    e->flags |= Velox::EntityFlags::Visible;

    g_font = Velox::GetAssetManager()->LoadFont("martius.ttf");

    if (g_font == nullptr)
        printf("font is null\n");

    g_fontShaderId = 0;

    while (!Velox::QuitRequested())
    {
        // GM: Has to be called start of every frame.
        // Can optionally use the returned value.
        float deltaTime = Velox::RefreshDeltaTime();

        Velox::Event event;
        while (Velox::PollEvents(&event))
        {
            HandleEvent(&event);
            // Do something with events.
        }        

        UpdateGame(deltaTime);

        Velox::StartFrame();
        
        DoRenderingStuff();

        Velox::EndFrame();
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
