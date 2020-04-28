#pragma once
#include "pch.h"
#include "Types.h"
#include "Texture.h"
#include "Util.h"
#include "Mesh.h"

//      Member structs      //

struct GameConfig {
    const U32 window_flags =
        SDL_WINDOW_BORDERLESS | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_OPENGL;

    const U32 fps = 60;
    const U32 frame_delay = 1000 / fps;

    float speed = 1.0f;
};

struct GamepadInput {
    static const U32 num_axes = SDL_CONTROLLER_AXIS_MAX;
    static const U32 num_buttons = SDL_CONTROLLER_BUTTON_MAX;
    static const S32 stick_deadzone_in = 8000;
    static const S32 stick_deadzone_out = 32767 - 1000;

    static U32 num_gamepads;

    SDL_GameController* sdl_ptr = nullptr;

    float axis[num_axes];
    Uint32 button_map, button_down_map, button_up_map;

    // NOTE: These could only be shifted by n-1 if SDL_GAMECONTROLLER_BUTTON
    // starts at 1
    inline bool button(U32 n) const { return button_map & BIT(n); };
    inline bool button_down(U32 n) const { return button_down_map & BIT(n); };
    inline bool button_up(U32 n) const { return button_up_map & BIT(n); };
};

struct MouseKeyboardInput {
    const static uint num_mouse_buttons = 3;
    const Uint8* sdl_keyboard;
    int num_keys;

    bool *key, *key_up, *key_down;
    uint mouse_button_map, mouse_button_down_map, mouse_button_up_map;
    glm::ivec2 mouse_pos;

    void init() {
        sdl_keyboard = SDL_GetKeyboardState(&num_keys);
        key = (bool*)malloc(sizeof(bool) * (num_keys * 3));
        key_down = key + num_keys;
        key_up = key_down + num_keys;
    };

    bool mouse_button_down(uint button) const {
        return mouse_button_down_map & button;
    };
};

struct RenderData {
    glm::ivec2 window_size = {1920, 1080};

    struct {
        GLuint id, model_matrix_loc, projection_matrix_loc;
    } simple_shader;

    struct {
        GLuint id, model_matrix_loc, projection_matrix_loc, bones_loc;
    } rigged_shader;

    struct {
        GLuint id, color_loc;
    } debug_shader;

    bool draw_models = true;
    bool draw_bones = true;
    bool draw_wireframes = false;

    glm::mat4 projection =
        glm::ortho(0.0f, 1920.0f, 0.0f, 1080.0f, -1.0f, 1.0f);

    void init(GLuint simple_shader_id, GLuint rigged_shader_id,
              GLuint debug_shader_id);
};

struct Player {
    glm::vec2 pos;
    Texture tex;
    RiggedMesh rigged_mesh;
    GamepadInput* gamepad_input;

    glm::mat4 model;
};

struct Game {
    bool running = false;
    U32 frame_start, last_frame_start;

    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_GLContext gl_context;

    GameConfig game_config;
    RenderData render_data;
    MouseKeyboardInput mouse_keyboard_input;
    Player player;

    std::array<GamepadInput, 4> gamepad_inputs;

    void init();
    bool run();
};