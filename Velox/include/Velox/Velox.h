#pragma once

namespace Velox {

struct EngineState {
    bool showPerformanceStats = false;
    bool showMemoryUsageStats = false;
    bool showSettings         = false;
};

// GM: Sets up all framework systems.
void Init();

EngineState* GetEngineState();

void DoFrameEndUpdates();

void Quit();
bool QuitRequested();

void DeInit();

}
