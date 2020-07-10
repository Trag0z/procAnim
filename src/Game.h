#pragma once
#include "pch.h"
#include "Types.h"
#include "Renderer.h"
#include "Texture.h"
#include "Util.h"
#include "Mesh.h"
#include "Player.h"
#include "Input.h"
#include "Background.h"
#include "Renderer.h"
#include "level/Level.h"

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

class Game {
public:
    bool running = false;
    U32 frame_start, last_frame_start;

    SDL_Window* window;
    SDL_Renderer* sdl_renderer;
    SDL_GLContext gl_context;

    GameConfig game_config;
    Renderer renderer;

    MouseKeyboardInput mouse_keyboard_input;
    std::array<Gamepad, Gamepad::NUM_PADS> gamepads;

    Player player;
    Background background;

    Level level;
    LevelEditor level_editor;

    enum GameMode { PLAY = 0, SPLINE_EDITOR = 1, LEVEL_EDITOR = 2 } game_mode;

    void init();
    bool run();

  private:
    void update_gui();
};