#pragma once

namespace Velox {

struct EngineState {
    bool showPerformanceStats = false;
    bool showMemoryUsageStats = false;
    bool showSettings         = false;
};

// GM: Sets up all framework systems.
void init();

EngineState* getEngineState();

void doFrameEndUpdates();

void quit();
bool quitRequested();

void deInit();

}
