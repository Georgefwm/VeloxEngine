#pragma once

#include <Velox.h>

namespace Velox { struct Event; }

struct ImDrawData;
SDL_EVENT_FWD_DECL

namespace Velox {

void initUI();

bool uiEventCallback(SDL_Event& event);

// Must be called after Velox::EndFrame()!
ImDrawData* getUIDrawData();

void deInitUI();

}
