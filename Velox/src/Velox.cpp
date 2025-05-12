#include "Velox.h"

#include <SDL3/SDL.h>
#include "SDL3/SDL_events.h"

#include <cstdio>

bool g_quitRequested = false;
SDL_Window* g_window = nullptr;

void Velox::test()
{
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS))
    {
        printf("Error: SDL_Init(): %s\n", SDL_GetError());
        return;
    }

    SDL_WindowFlags windowFlags = SDL_WINDOW_VULKAN | SDL_WINDOW_HIGH_PIXEL_DENSITY;
    g_window = SDL_CreateWindow("Velox App", 1280, 720, windowFlags);
    if (g_window == nullptr)
    {
        printf("Error: SDL_CreateWindow(): %s\n", SDL_GetError());
        return;
    }

    // Main loop.
    while (!g_quitRequested)
    {
        // Handle events.
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_EVENT_QUIT)
                g_quitRequested = true;
        }

        // Update game state.

        // Render.
    }
    
    SDL_Quit();
}
