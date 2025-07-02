#pragma once

#include "Velox.h"

namespace Velox { struct Event; }

struct ImDrawData;

namespace Velox {

void initUI();

void forwardSDLEventToUI(Velox::Event* event);

// Must be called after Velox::EndFrame()!
ImDrawData* getUIDrawData();

void deInitUI();

}
