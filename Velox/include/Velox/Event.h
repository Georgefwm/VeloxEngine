#pragma once

#include <Velox.h>

#include "SDL3/SDL_events.h"

namespace Velox {

enum EventType {
    SDLEvent,
    AssetLoadRequest,
};

struct SubscribeInfo {
    const char* name;
    u32 eventRangeStart = SDL_EVENT_FIRST;
    u32 eventRangeEnd   = SDL_EVENT_LAST;
    std::function<bool(SDL_Event&)> callback;  // bool recieveEvent(SDL_Event event);
    i32 priority = 10;  // default 10 (semi-low priority).
};

struct EventPublisher {
    std::vector<SubscribeInfo> subscribers;
    void processEvents();
    void subscribe(const SubscribeInfo& info);
    void printSubscribers();
};

void initEvents();
EventPublisher* getEventPublisher();

}
