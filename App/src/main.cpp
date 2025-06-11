#include "Asset.h"
#include "Event.h"

#include "Rendering/Renderer.h"
#include "Velox.h"
#include "Entity.h"
#include "Primitive.h"
#include "Util.h"

#include <imgui.h>
#include <iostream>
#include <ostream>

Velox::EntityManager g_entityManager;;

Velox::EntityHandle e1;
float direction = 1.0;

void HandleEvent(Velox::Event* event)
{

}

void UpdateGame(float deltaTime)
{
    Velox::Entity* e = g_entityManager.getMut(e1);

    e->position.x += (300 * direction) * deltaTime;

    ivec2 windowSize = Velox::GetWindowSize();
    if (e->position.x > windowSize.x * 0.95) direction = -1.0;
    if (e->position.x < windowSize.x * 0.05) direction =  1.0;
}

void DoRenderingStuff()
{
    Velox::DrawRectangle(vec4(200, 200, 200, 200), vec4(1.0, 0.0, 0.0, 1.0), -1); // Untextured
    Velox::DrawRectangle(vec4(200, 500, 200, 200), vec4(1.0, 1.0, 1.0, 1.0),  0); // Textured

    Velox::Entity* e = g_entityManager.getMut(e1);
    e->Draw(true);

    Velox::DrawRectangle(vec4(700, 200, 200, 200), vec4(1.0, 1.0, 1.0, 1.0),  0); // Textured

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
    e->textureIndex = Velox::GetAssetManager()->LoadTexture("star.png");
    e->flags |= Velox::EntityFlags::Visible;

    // for (auto [handle, entity] : entityManager.iter())
    // {
    //     printf("Iterator test 1: %i\n", entity->test);
    // }

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
