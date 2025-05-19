#include "Velox.h"

#include "UI.h"
#include "Renderer.h"

#include <SDL3/SDL.h>

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

void Velox::Quit()
{
    g_quitRequested = true;
}

bool Velox::QuitRequested()
{
    return g_quitRequested;
}

void Velox::DeInit()
{
    Velox::DeInitRenderer();
    Velox::DeInitUI();

    SDL_Quit();    
}

