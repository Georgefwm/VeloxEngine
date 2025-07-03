#pragma once

#include "Velox.h"

#include <imgui.h>

namespace Velox {

struct VELOX_API EngineState {
    bool showPerformanceStats = false;
    bool showMemoryUsageStats = false;
    bool showSettings         = false;
    bool showEntityInfo       = false;
};

// GM: Sets up all framework systems.
VELOX_API void init();

VELOX_API EngineState* getEngineState();

VELOX_API void doFrameEndUpdates();

VELOX_API void quit();
VELOX_API bool quitRequested();

VELOX_API void deInit();

}
