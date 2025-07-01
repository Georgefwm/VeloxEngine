#include "Event.h"
#include <PCH.h>

#include "Console.h"
#include "Rendering/Renderer.h"
#include "SDL3/SDL_events.h"
#include "UI.h"
#include "Velox.h"

#include <queue>

static std::queue<Velox::Event> g_eventQueue;

void Velox::pushEvent(Velox::Event event)
{
    g_eventQueue.push(event);
}

// GM: Bit janky rn, currently we just intercept events we want to use.
bool Velox::pollEvents(Velox::Event* event)
{
    Event* t_event;

    while (!g_eventQueue.empty())
    {
        t_event = &g_eventQueue.front();

        if (shouldEngineInterceptEvent(t_event))
        {
            if (interceptEvent(t_event))
            {
                event = t_event;
                g_eventQueue.pop();
                return true;
            }
        }
        else
        {
            event = t_event;
            g_eventQueue.pop();
            return true;
        }
    }

    // GM: Ideally we would just extract information from SDL_Events that are interesting and 
    // create purely native velox event, but that seems like a lot of grunt work. We also need
    // to sus out where our math types are going to come from (lib or handmade).
    SDL_Event sdlEvent;
    while (SDL_PollEvent(&sdlEvent))
    {
        Velox::Event newEvent = { Velox::SDLEvent, sdlEvent };
        t_event = &newEvent;

        if (shouldEngineInterceptEvent(t_event))
        {
            if (interceptEvent(t_event))
            {
                event = t_event;
                return true;
            }
        }
        else 
        {
            event = t_event;
            return true;
        }
    }

    return false;
}

bool Velox::shouldEngineInterceptEvent(Velox::Event* event)
{
    if (event->type == Velox::EventType::AssetLoadRequest)
        return true;

    if (event->type == Velox::EventType::SDLEvent)
        return true;

    return false;
}

// Returns true if event should propogate to user.
bool Velox::interceptEvent(Velox::Event* event)
{
    if (event->type == Velox::EventType::SDLEvent)
    {
        if (event->sdlEvent.type == SDL_EVENT_QUIT)
        {
            // TODO: Let the developer handle the quit logic.
            Velox::quit();
            return false;
        }

        if (event->sdlEvent.type == SDL_EVENT_KEY_DOWN)
        {
            if (event->sdlEvent.key.scancode == SDL_SCANCODE_GRAVE) 
            {
                // Set this as a button that never gets through to the user (engine reserved).
                Velox::toggleConsole();
                return false;
            }
        }
        
        // SDL window events fall between this range.
        // The renderer only needs to know about window events.
        if (event->sdlEvent.type >= Uint32(0x200) && event->sdlEvent.type < Uint32(0x300))
        {
            Velox::forwardSDLEventToRenderer(&event->sdlEvent);
            return true;
        }

        Velox::forwardSDLEventToUI(event);

        return true;
    }

    return true;
}
