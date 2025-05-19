#pragma once

namespace Velox { struct Event; }
struct ImDrawData;

namespace Velox {

void InitUI();

void ForwardSDLEvent(Velox::Event* event);

// Must be called after Velox::EndFrame()!
ImDrawData* GetUIDrawData();

void DeInitUI();

}
