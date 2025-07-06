#include "Core.h"
#include <PCH.h>

#include "Asset.h"
#include "Config.h"
#include "Console.h"
#include "Debug.h"
#include "Entity.h"
#include "Event.h"
#include "Input.h"
#include "Text.h"
#include "Timing.h"
#include "UI.h"
#include "Rendering/Renderer.h"

#include <SDL3/SDL.h>
#include <SDL3/SDL_time.h>
#include <SDL3/SDL_timer.h>
#include <imgui.h>
#include <imgui_impl_sdl3.h>

bool g_quitRequested = false;

static Velox::EngineState engineState {};

SDL_Time timeStamp(std::string label, SDL_Time& startTime)
{
    SDL_Time time;
    SDL_GetCurrentTime(&time);

    time = SDL_NS_TO_MS(time - startTime);
    LOG_INFO("{}: {}ms", label, std::to_string(time));

    return time;   
}

void Velox::init()
{
    Velox::initLog();
    LOG_TRACE("Initialising engine...");

    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS))
    {
        LOG_CRITICAL("Failed to initialise SDL: {}", SDL_GetError());
        throw std::runtime_error("");
    }

    SDL_Time initStartTime;
    SDL_GetCurrentTime(&initStartTime);

#define SPLIT_TIMES 0


    Velox::initEvents();
#if SPLIT_TIMES
    timeStamp("Events", initStartTime);
#endif

    bool userConfigExists;
    Velox::initConfig(&userConfigExists);
#if SPLIT_TIMES
    timeStamp("Config", initStartTime);
#endif

    Velox::initAssets();
#if SPLIT_TIMES
    timeStamp("Assets", initStartTime);
#endif

    Velox::initRenderer();
#if SPLIT_TIMES
    timeStamp("Renderer", initStartTime);
#endif

    Velox::initText();
#if SPLIT_TIMES
    timeStamp("Text", initStartTime);
#endif

    Velox::initUI();
#if SPLIT_TIMES
    timeStamp("UI", initStartTime);
#endif

    Velox::initConsole();
#if SPLIT_TIMES
    timeStamp("Console", initStartTime);
#endif

    Velox::initTimer();
#if SPLIT_TIMES
    timeStamp("Timer", initStartTime);
#endif

    Velox::initEntitySystem();
#if SPLIT_TIMES
    timeStamp("Entity System", initStartTime);
#endif

    Velox::initInput();
#if SPLIT_TIMES
    timeStamp("Input", initStartTime);
#endif

    Velox::SubscribeInfo subInfo {
        .name = "Core",
        .eventRangeStart = SDL_EVENT_QUIT,
        .eventRangeEnd   = SDL_EVENT_QUIT,
        .callback = Velox::coreEventCallback,
        .priority = 1,
    };

    Velox::getEventPublisher()->subscribe(subInfo);


    SDL_Time initEndTime;
    SDL_GetCurrentTime(&initEndTime);

    std::string initTimeString = fmt::format("Engine initialised, took {}ms",
            std::to_string(SDL_NS_TO_MS(initEndTime - initStartTime)));

    Velox::printToConsole(initTimeString);
    LOG_TRACE(initTimeString);
}

bool Velox::coreEventCallback(SDL_Event& event)
{
    // TODO: Let the developer handle the quit logic.
    Velox::quit();
    return false;
}

Velox::EngineState* Velox::getEngineState()
{
    return &engineState;
}

void Velox::doFrameEndUpdates()
{
    Velox::drawConsole();

    if (engineState.showPerformanceStats)
        Velox::drawPerformanceStats();

    if (engineState.showMemoryUsageStats)
        Velox::drawMemoryUsageStats();

    if (engineState.showSettings)
        Velox::drawSettings();

    if (engineState.showEntityInfo)
        Velox::drawEntityHierarchyInfo();

    if (engineState.drawColliders)
        Velox::drawEntityColliders();

    Velox::updateKeyStates();
}

void Velox::quit()
{
    g_quitRequested = true;
}

bool Velox::quitRequested()
{
    return g_quitRequested;
}

void Velox::deInit()
{
    Velox::deInitAssets(); // Must be cleaned up before GLContext is destroyed (in deInitRenderer()). (I think...)

    Velox::deInitRenderer();
    Velox::deInitUI();

    SDL_Quit();    
}

