#include "Event.h"

#include "Renderer.h"
#include "Velox.h"
#include "Entity.h"
#include "Primitive.h"
#include "Util.h"

#include <cstdio>
#include <imgui.h>
#include <iostream>
#include <ostream>

void HandleEvent(Velox::Event* event)
{

}

void UpdateGame()
{

}

void DoRenderingStuff()
{
    Velox::DrawRectangle(vec4(200, 200, 500, 200), vec4(1.0, 0.0, 0.0, 1.0), -1); // Untextured
    Velox::DrawRectangle(vec4(200, 500, 500, 200), vec4(1.0, 1.0, 1.0, 1.0),  0); // Textured

    // ImGui::ShowDemoWindow();
}

void run()
{
    Velox::Init();

    // GM: I guess when done like this, entities are allocated on the stack? ...maybe.
    Velox::EntityManager entityManager {};

    // GM: Will make a tests file, just doing this for now.
    // Test start.
    if (false)
    {
        Velox::EntityHandle handle = entityManager.createEntity();
        Velox::Entity* newEntity = entityManager.getMut(handle);

        printf("before modification: %i\n", newEntity->test);

        newEntity->position.x += 1;
        newEntity->test += 1;

        printf("after modification: %i\n", newEntity->test);

        newEntity = entityManager.getMut(handle);

        printf("after modification (re-query): %i\n", newEntity->test);

        printf("\n");
        for (auto [handle, entity] : entityManager.iter())
        {
            printf("Iterator test 1: %i\n", entity->test);
        }

        entityManager.destroyEntity(handle);

        entityManager.createEntity();
        entityManager.createEntity();

        printf("\n");
        for (auto [handle, entity] : entityManager.iter())
        {
            printf("Iterator test 2: %i\n", entity->test);
        }

        Velox::EntityHandle newHandle = entityManager.createEntity();
        newEntity = entityManager.getMut(newHandle);

        entityManager.createEntity();

        printf("\n");
        for (auto [handle, entity] : entityManager.iter())
        {
            printf("Iterator test 3: %i\n", entity->test);
        }

        newEntity->test += 1;

        printf("\n");
        for (auto [handle, entity] : entityManager.iter())
        {
            printf("Iterator test 4: %i\n", entity->test);
        }
        // Test end.
    }

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

        UpdateGame();

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
