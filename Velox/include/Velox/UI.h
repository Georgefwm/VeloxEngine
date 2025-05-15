#pragma once

class ImDrawData;

namespace Velox {

void InitUI();

// Must be called after Velox::EndFrame()!
ImDrawData* GetUIDrawData();

void DeInitUI();

}
