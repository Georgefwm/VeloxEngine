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

    void PushEvent(Event event);
    bool PollEvents(Event* event);

    bool ShouldEngineInterceptEvent(Event* event);
    bool InterceptEvent(Event* event);
}
