#include "Velox.h"
#include <PCH.h>

#include "Asset.h"
#include "Config.h"
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

bool g_quitRequested = false;

static Velox::EngineState engineState {};

SDL_Time TimeStamp(std::string label, SDL_Time& startTime)
{
    SDL_Time time;
    SDL_GetCurrentTime(&time);

    time = SDL_NS_TO_MS(time - startTime);
    fmt::println(fmt::format("{}: {}ms", label, std::to_string(time)));

    return time;   
}

void Velox::Init()
{
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS))
    {
        printf("Error: SDL_Init(): %s\n", SDL_GetError());
        return;
    }

    SDL_Time initStartTime;
    SDL_GetCurrentTime(&initStartTime);

#define SPLIT_TIMES 0

    bool userConfigExists;
    Velox::InitConfig(&userConfigExists);
#if SPLIT_TIMES
    TimeStamp("Config", initStartTime);
#endif

    // if (!userConfigExists)
    //     perform some first time setup

    Velox::InitAssets();
#if SPLIT_TIMES
    TimeStamp("Assets", initStartTime);
#endif

    Velox::InitRenderer();
#if SPLIT_TIMES
    TimeStamp("Renderer", initStartTime);
#endif

    Velox::InitText();
#if SPLIT_TIMES
    TimeStamp("Text", initStartTime);
#endif

    Velox::InitUI();
#if SPLIT_TIMES
    TimeStamp("UI", initStartTime);
#endif

    Velox::InitConsole();
#if SPLIT_TIMES
    TimeStamp("Console", initStartTime);
#endif

    SDL_Time initEndTime;
    SDL_GetCurrentTime(&initEndTime);

    std::string initTimeString = fmt::format("Engine initialised, took {}ms",
            std::to_string(SDL_NS_TO_MS(initEndTime - initStartTime)));

    Velox::PrintToConsole(initTimeString);
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

    if (engineState.showSettings)
        Velox::DrawSettings();
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
    Velox::DeInitAssets(); // Must be cleaned up before GLContext is destroyed (in DeInitRenderer()). (I think...)

    Velox::DeInitRenderer();
    Velox::DeInitUI();

    SDL_Quit();    
}

