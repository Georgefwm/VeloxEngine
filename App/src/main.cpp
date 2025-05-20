#include "Event.h"
#include "Renderer.h"
#include "Velox.h"
#include "Entity.h"

#include <cstdio>
#include <imgui.h>

void HandleEvent(Velox::Event* event)
{

}

void UpdateGame()
{

}

void DoRenderingStuff()
{
    ImGui::ShowDemoWindow();
}

int main(int argc, char** argv)
{
    Velox::Init();

    // GM: I guess when done like this, entities are allocated on the stack? ...maybe.
    Velox::EntityManager entityManager {};

    // GM: Will make a tests file, just doing this for now.
    // Test start.

    Velox::EntityHandle handle = entityManager.createEntity();
    Velox::Entity* newEntity = entityManager.getMut(handle);

    printf("before modification: %i\n", newEntity->test);

    newEntity->test = 42;

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

    newEntity->test = 69;

    printf("\n");
    for (auto [handle, entity] : entityManager.iter())
    {
        printf("Iterator test 4: %i\n", entity->test);
    }
    // Test end.

    while (!Velox::QuitRequested())
    {
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
        Velox::DoRenderPass();
    }

    // Cleanup.
    Velox::DeInit();

    return 0;
}
