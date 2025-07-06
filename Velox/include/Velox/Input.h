#pragma once

#include <Velox.h>

SDL_EVENT_FWD_DECL

#include <SDL3/SDL_scancode.h>

namespace Velox {

enum KeyState : u8 {
    Pressed  = 1 << 1,  // Pressed this frame.
    Down     = 1 << 2,  // Currently pressed.
};

void initInput();

void updateKeyStates();

bool isKeyPressed(SDL_Scancode code);
bool isKeyDown(SDL_Scancode code);

bool inputEventCallback(SDL_Event& event);

}
