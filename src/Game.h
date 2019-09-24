#pragma once
#include "Components.h"
#include "Shader.h"
#include <sdl/SDL.h>
#include <array>

struct Game {
    bool running = false;
    U32 frameStart;

    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_GLContext gContext;
    Shader shader;

    GameConfig gameConfig;
    MouseKeyboardInput mouseKeyboardInput;

    std::array<GamepadInput, 4> gamepadInputs;

    Transform transform;
    SpriteRenderer spriteRenderer;
    FlatRenderer flatRenderer;

    void init();
    bool run();
};