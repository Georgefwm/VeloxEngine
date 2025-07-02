#pragma once

#include "Velox.h"

#include "SDL3/SDL_events.h"

namespace Velox {

enum EventType {
    SDLEvent,
    AssetLoadRequest,
};

struct AssetLoadRequest {
    const char* filepath;
};

struct Event {
    EventType type;
    union {
        SDL_Event sdlEvent;
        struct AssetLoadRequest loadEvent;
    };
};

VELOX_API void pushEvent(Event event);
VELOX_API bool pollEvents(Event* event);

bool shouldEngineInterceptEvent(Event* event);
bool interceptEvent(Event* event);
}
