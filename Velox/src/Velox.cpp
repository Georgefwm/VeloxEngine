#include "Velox.h"
#include <PCH.h>

#include "Asset.h"
#include "Console.h"
#include "Debug.h"
#include "UI.h"
#include "Rendering/Renderer.h"

#include <SDL3/SDL.h>

#include <imgui.h>
#include <imgui_impl_sdl3.h>

bool g_quitRequested = false;

static Velox::EngineState engineState {};

void Velox::Init()
{
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS))
    {
        printf("Error: SDL_Init(): %s\n", SDL_GetError());
        return;
    }

    Velox::InitRenderer();
    Velox::InitAssets();
    Velox::InitUI();
    Velox::InitConsole();
}

Velox::EngineState* Velox::GetEngineState()
{
    return &engineState;
}

void Velox::DoFrameEndUpdates()
{
    Velox::DrawConsole();

    if (engineState.showPerformanceStats)
        Velox::DrawPerformanceStats();

    if (engineState.showMemoryUsageStats)
        Velox::DrawMemoryUsageStats();
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

