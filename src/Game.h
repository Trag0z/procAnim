#pragma once
#include "pch.h"
#include "Types.h"
#include "Renderer.h"
#include "Texture.h"
#include "Util.h"
#include "Mesh.h"
#include "Collider.h"
#include "Player.h"
#include "Input.h"

class Player;

//----------------------------------//
//----------Member structs----------//
//----------------------------------//

struct GameConfig {
    const U32 window_flags =
        SDL_WINDOW_BORDERLESS | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_OPENGL;

    const U32 fps = 60;
    const U32 frame_delay = 1000 / fps;

    float speed = 1.0f;
    bool step_mode = false;
    bool use_const_delta_time = true;
};

//-------------------------------//
//----------Game struct----------//
//-------------------------------//

struct Game {
    bool running = false;
    U32 frame_start, last_frame_start;

    SDL_Window* window;
    SDL_Renderer* sdl_renderer;
    SDL_GLContext gl_context;

    GameConfig game_config;
    Renderer renderer;

    MouseKeyboardInput mouse_keyboard_input;
    std::array<Gamepad, Gamepad::num_pads> gamepads;

    Player player;
    BoxCollider ground;

    void init();
    bool run();
};