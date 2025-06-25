#include "Velox.h"
#include <PCH.h>

#include "Asset.h"
#include "Console.h"
#include "Debug.h"
#include "Text.h"
#include "UI.h"
#include "Rendering/Renderer.h"

#include <SDL3/SDL.h>

#include <SDL3/SDL_time.h>
#include <SDL3/SDL_timer.h>
#include <imgui.h>
#include <imgui_impl_sdl3.h>

#include <format>

bool g_quitRequested = false;

static Velox::EngineState engineState {};

void Velox::Init()
{
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS))
    {
        printf("Error: SDL_Init(): %s\n", SDL_GetError());
        return;
    }

    SDL_Time initStartTime;
    SDL_GetCurrentTime(&initStartTime);

    Velox::InitAssets();
    Velox::InitRenderer();
    Velox::InitText();
    Velox::InitUI();
    Velox::InitConsole();

    SDL_Time initEndTime;
    SDL_GetCurrentTime(&initEndTime);

    SDL_Time t = SDL_NS_TO_MS(initEndTime) - SDL_NS_TO_MS(initStartTime);
    std::string initString = "Engine initialiased, took ";
    initString += std::to_string(t);
    initString += " ms";

    Velox::PrintToConsole(initString);
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
    Velox::DeInitAssets(); // Must be cleaned up before GLContext is destroyed (in DeInitRenderer()).

    Velox::DeInitRenderer();
    Velox::DeInitUI();

    SDL_Quit();    
}

