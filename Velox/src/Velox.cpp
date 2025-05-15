#include "Velox.h"

#include "UI.h"
#include "Renderer.h"

#include <SDL3/SDL.h>
#include "SDL3/SDL_events.h"

#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlgpu3.h>

#include <cstdio>

bool g_quitRequested = false;

void Velox::Init()
{
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS))
    {
        printf("Error: SDL_Init(): %s\n", SDL_GetError());
        return;
    }

    Velox::InitRenderer();
    Velox::InitUI();
}

void Velox::DeInit()
{
    Velox::DeInitRenderer();
    Velox::DeInitUI();

    SDL_Quit();    
}

void Velox::test()
{
    Velox::Init();

    // Main loop.
    while (!g_quitRequested)
    {
        // TODO: Add our own event system to expose to user.
        // Handle events.
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            ImGui_ImplSDL3_ProcessEvent(&event);

            if (event.type == SDL_EVENT_QUIT)
                g_quitRequested = true;
        }
        
        // GM: Reduce framerate while window is minimised.
        // if (SDL_GetWindowFlags(g_window) & SDL_WINDOW_MINIMIZED)
        // {
        //     SDL_Delay(10);
        // }

        // Update game state.
        // No game yet...

        // Render.

        Velox::StartFrame();

        ImGui::ShowDemoWindow();

        Velox::EndFrame();

        Velox::DoRenderPass();
    }

    // Cleanup.
    Velox::DeInit();
}
