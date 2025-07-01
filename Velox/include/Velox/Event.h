#pragma once

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

    void pushEvent(Event event);
    bool pollEvents(Event* event);

    bool shouldEngineInterceptEvent(Event* event);
    bool interceptEvent(Event* event);
}
