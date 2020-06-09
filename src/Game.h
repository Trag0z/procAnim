#pragma once
#include "pch.h"
#include "Types.h"
#include "Texture.h"
#include "Util.h"
#include "Mesh.h"
#include "Collider.h"
#include "Player.h"
#include "Spline.h"
#include "Input.h"
#include "Player.h"

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

struct RenderData {
    glm::ivec2 window_size = {1920, 1080};

    struct {
        GLuint id;
    } simple_shader;

    struct {
        GLuint id, projection_loc, model_loc;
    } rigged_shader;

    struct {
        GLuint id, projection_loc, color_loc, model_loc;
    } debug_shader;

    bool draw_models = true;
    bool draw_bones = true;
    bool draw_wireframes = false;
    bool draw_splines = true;
    bool draw_circles = false;

    glm::mat4 projection =
        glm::ortho(0.0f, 1920.0f, 0.0f, 1080.0f, -1.0f, 1.0f);

    void init(GLuint simple_shader_id, GLuint rigged_shader_id,
              GLuint debug_shader_id);
};

//-------------------------------//
//----------Game struct----------//
//-------------------------------//

struct Game {
    bool running = false;
    U32 frame_start, last_frame_start;

    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_GLContext gl_context;

    GameConfig game_config;
    RenderData render_data;

    MouseKeyboardInput mouse_keyboard_input;
    std::array<Gamepad, Gamepad::num_pads> gamepad_inputs;

    Player player;
    BoxCollider ground;

    void init();
    bool run();
};