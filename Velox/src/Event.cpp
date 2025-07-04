#include "Event.h"
#include <PCH.h>

#include "Console.h"
#include "Rendering/Renderer.h"
#include "Core.h"


static Velox::EventPublisher s_publisher {};

void Velox::EventPublisher::processEvents()
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        for (Velox::SubscribeInfo& sub : subscribers)
        {
            if (sub.eventRangeStart > event.type || sub.eventRangeEnd < event.type)
                continue;

            bool consumed = sub.callback(event);

            if (consumed)
                break;
        }
    }
}

void Velox::EventPublisher::subscribe(const SubscribeInfo& info)
{
    if (subscribers.empty())
    {
        subscribers.push_back(info);
        LOG_TRACE("EventPublisher: '{}' subcribed with priority {}", info.name, info.priority);
        return;
    }

    auto insertPos = subscribers.begin();
    for (i32 i = 0; i < subscribers.size(); i++)
    {
        if (info.priority < subscribers[i].priority)
            break;
        
        insertPos += 1;
    }

    LOG_TRACE("EventPublisher: '{}' subcribed with priority {}", info.name, info.priority);
    subscribers.insert(insertPos, info);
}

void Velox::EventPublisher::printSubscribers()
{
    LOG_INFO("Current Subs:");
    for (SubscribeInfo& sub : subscribers)
    {
        LOG_INFO("SUB - priority: {}, name: {}", sub.priority, sub.name);
    }
}

void Velox::initEvents()
{

}

Velox::EventPublisher* Velox::getEventPublisher() { return &s_publisher; }

