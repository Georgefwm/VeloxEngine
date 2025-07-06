#include "Input.h"
#include "Event.h"
#include <PCH.h>

static std::unordered_map<SDL_Scancode, u8> s_keyStates;

bool checkState(const u32& value, Velox::KeyState state)
{
    return (value & state) != 0;
}

// TODO: Come up with better vairable names, this is confusing.
void setState(u8& value, Velox::KeyState state, bool desiredState)
{
    if (desiredState) value |=  state;
    else              value &= ~state;
}

void Velox::initInput()
{
    Velox::SubscribeInfo subInfo {
        .name = "Input",
        .eventRangeStart = SDL_EVENT_KEY_DOWN,
        .eventRangeEnd   = SDL_EVENT_KEY_UP,
        .callback = Velox::inputEventCallback,
        .priority = 4,
    };
}

void Velox::updateKeyStates()
{
    // Reset pressed this frame states.
    for (auto& pair : s_keyStates)
        setState(pair.second, Velox::KeyState::Pressed, false);
}

bool Velox::isKeyPressed(SDL_Scancode code)
{
    if (s_keyStates.find(code) == s_keyStates.end())
        return false;

    return checkState(s_keyStates[code], Velox::KeyState::Pressed);
}

bool Velox::isKeyDown(SDL_Scancode code)
{
    if (s_keyStates.find(code) == s_keyStates.end())
        return false;

    return checkState(s_keyStates[code], Velox::KeyState::Down);
}

bool Velox::inputEventCallback(SDL_Event& event)
{
    if (event.type == SDL_EVENT_KEY_UP)
    {
        s_keyStates[event.key.scancode] = 0;
        return false;
    }

    s_keyStates[event.key.scancode] = Velox::KeyState::Down | Velox::KeyState::Pressed;
    return false;
}

