#pragma once
#include "Components.h"
#include "Entity.h"
#include <array>
#include <sdl/SDL.h>

struct Game {
    bool running = false;
    U32 frameStart;

    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_GLContext glContext;
    Texture tex;

    GameConfig gameConfig;
    MouseKeyboardInput mouseKeyboardInput;

    std::array<GamepadInput, 4> gamepadInputs;

    Entity entity;

    void init();
    bool run();
};