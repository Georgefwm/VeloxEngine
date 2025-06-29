#include "Timing.h"
#include <PCH.h>

#include "Rendering/Renderer.h"
#include "RingBuffer.h"
#include <SDL3/SDL_timer.h>
#include <SDL3/SDL_video.h>
#include <functional>
#include <glm/common.hpp>

static i64 s_currentFrameTime = 0;
static i64 s_currentDeltaTime = 0;

static bool s_resync = true;

static f64  s_updateRate;
static i32  s_updateMultiplicity;
static bool s_unlockFramerate;

static i64 s_clocksPerSecond;
static f64 s_fixedDeltaTime;
static i64 s_desiredFrameTime;

static i64 s_vsyncMaxError;

static i32 s_displayFrameRate;
static SDL_DisplayMode* s_currentDisplayMode;
static i64 s_snapHz;

static std::vector<i64> s_snapFrequencies;

static constexpr u32 TIME_HISTORY_COUNT = 4;
static Velox::RingBuffer<i64> s_timeAverager(TIME_HISTORY_COUNT);
static i64 s_averagerResidual = 0;

static i64 s_previousFrameTime;
static i64 s_frameAccumulator;

f64 Velox::DeltaTime() { return s_fixedDeltaTime; }

void Velox::CalculateDeltaTime()
{
    s_currentFrameTime  = SDL_GetPerformanceCounter();
    s_currentDeltaTime  = s_currentFrameTime - s_previousFrameTime;
    s_previousFrameTime = s_currentFrameTime;
    
    // Set limit for overflow, long frames, etc.
    s_currentDeltaTime = glm::clamp(s_currentDeltaTime, (i64)0, s_desiredFrameTime * 8);

    // Snap to vsync time.
    for (i64 snap : s_snapFrequencies)
    {
        if (glm::abs(s_currentDeltaTime - snap) < s_vsyncMaxError)
            s_currentDeltaTime = snap;
    }

    s_timeAverager.Push(s_currentDeltaTime);

    i64 averagerSum = 0;
    for (int i = 0; i < s_timeAverager.Size(); i++)
        averagerSum += s_timeAverager[i];

    s_currentDeltaTime  = averagerSum / TIME_HISTORY_COUNT;

    s_averagerResidual += averagerSum        % TIME_HISTORY_COUNT;
    s_currentDeltaTime += s_averagerResidual / TIME_HISTORY_COUNT;
    s_averagerResidual %= TIME_HISTORY_COUNT;

    s_frameAccumulator += s_currentDeltaTime;

    if (s_frameAccumulator > s_desiredFrameTime * 8)
        s_resync = true;

    if (s_resync)
    {
        s_frameAccumulator = s_desiredFrameTime * 2;
        s_currentDeltaTime = s_desiredFrameTime;
        s_timeAverager.Assign(s_desiredFrameTime);

        s_resync = false;
    }
}

void Velox::UpdateGame(std::function<void(double&)> updateCallback)
{
    while (s_frameAccumulator >= s_desiredFrameTime * s_updateMultiplicity)
    {
        for (int i = 0; i < s_updateMultiplicity; i++)
        {
            updateCallback(s_fixedDeltaTime);
            s_frameAccumulator -= s_desiredFrameTime;
        }
    }
}

void Velox::InitTimer()
{
    // For now just set update rate to screen refresh rate (force vsync on).
    // Seem like relatively soon, SDL3 will implement frame pacing extension or allow for DXGI
    // adapter for opengl on windows.
    //
    // Reference:
    // https://github.com/libsdl-org/SDL/issues/10160
    SDL_DisplayID displayID = SDL_GetDisplayForWindow(Velox::GetWindow());
    const SDL_DisplayMode* currentDisplayMode = SDL_GetCurrentDisplayMode(displayID);

    // Read from config file
    s_updateRate         = currentDisplayMode->refresh_rate;
    s_updateMultiplicity = 1;
    s_unlockFramerate    = false;

    s_clocksPerSecond  = SDL_GetPerformanceFrequency();
    s_fixedDeltaTime   = 1.0 / s_updateRate;
    s_desiredFrameTime = s_clocksPerSecond / s_updateRate;

    s_vsyncMaxError = s_clocksPerSecond * 0.0002;

    s_displayFrameRate = 60;

    // SDL_DisplayID displayID = SDL_GetDisplayForWindow(Velox::GetWindow());
    // const SDL_DisplayMode* currentDisplayMode = SDL_GetCurrentDisplayMode(displayID);
    s_displayFrameRate = currentDisplayMode->refresh_rate_numerator;
    s_snapHz = s_displayFrameRate;

    // If the monitor is 59.94 but the target update is 60, just snap vsync'd delta times to 60.
    if (abs(s_displayFrameRate - s_updateRate) < 0.1) 
        s_snapHz = s_updateRate;

    for(int i = 0; i < s_snapFrequencies.size(); i++)
        s_snapFrequencies[i] = (s_clocksPerSecond / s_snapHz) * (i + 1);
    
    s_timeAverager.Assign(s_desiredFrameTime);
    s_averagerResidual = 0;

    s_previousFrameTime = SDL_GetPerformanceCounter();
    s_frameAccumulator = 0;
}
