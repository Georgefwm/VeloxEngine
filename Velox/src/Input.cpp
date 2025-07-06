#include "Input.h"
#include "Event.h"
#include "Rendering/Renderer.h"
#include <PCH.h>

static std::unordered_map<SDL_Scancode, u8> s_keyStates;
static std::unordered_map<u8, u8> s_mouseButtonStates;


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
    Velox::SubscribeInfo keyboardSubInfo {
        .name = "Keyboard Input",
        .eventRangeStart = SDL_EVENT_KEY_DOWN,
        .eventRangeEnd   = SDL_EVENT_KEY_UP,
        .callback = Velox::keyboardInputEventCallback,
        .priority = 4,
    };

    Velox::getEventPublisher()->subscribe(keyboardSubInfo);

    Velox::SubscribeInfo mouseSubInfo {
        .name = "Mouse Input",
        .eventRangeStart = SDL_EVENT_MOUSE_BUTTON_DOWN,
        .eventRangeEnd   = SDL_EVENT_MOUSE_BUTTON_UP,
        .callback = Velox::mouseInputEventCallback,
        .priority = 4,
    };

    Velox::getEventPublisher()->subscribe(mouseSubInfo);
}

// Reset pressed this frame states.
void Velox::updateKeyStates()
{
    for (auto &pair : s_mouseButtonStates)
        setState(pair.second, Velox::KeyState::Pressed, false);

    for (auto &pair : s_keyStates)
        setState(pair.second, Velox::KeyState::Pressed, false);
}

vec2 Velox::getMousePosition()
{
    float x, y;
    SDL_GetMouseState(&x, &y);
    y = Velox::getWindowSize().y - y;

    return vec2(x, y);
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

bool Velox::isMouseButtonPressed(u8 button)
{
    if (s_mouseButtonStates.find(button) == s_mouseButtonStates.end())
        return false;

    return checkState(s_mouseButtonStates[button], Velox::KeyState::Pressed);
}

bool Velox::isMouseButtonDown(u8 button)
{
    if (s_mouseButtonStates.find(button) == s_mouseButtonStates.end())
        return false;

    return checkState(s_mouseButtonStates[button], Velox::KeyState::Down);
}

bool Velox::keyboardInputEventCallback(SDL_Event& event)
{
    if (event.type == SDL_EVENT_KEY_UP)
    {
        s_keyStates[event.key.scancode] = 0;
        return false;
    }

    s_keyStates[event.key.scancode] = Velox::KeyState::Down | Velox::KeyState::Pressed;
    return false;
}

bool Velox::mouseInputEventCallback(SDL_Event& event)
{
    if (event.type == SDL_EVENT_MOUSE_BUTTON_UP)
    {
        s_mouseButtonStates[event.button.button] = 0;
        return false;
    }

    s_mouseButtonStates[event.button.button] = Velox::KeyState::Down | Velox::KeyState::Pressed;
    return false;
}

