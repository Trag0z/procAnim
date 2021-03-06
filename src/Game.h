#pragma once
#include "Types.h"
#include "rendering/Renderer.h"
#include "rendering/Texture.h"
#include "Util.h"
#include "rendering/Mesh.h"
#include "Player.h"
#include "Input.h"
#include "Background.h"
#include "rendering/Renderer.h"
#include "Level.h"
#include "Audio.h"
#include "ConfigManager.h"
#include "Ball.h"
#include <sdl\SDL.h>

namespace Keybinds {
constexpr SDL_Scancode DRAW_BONES   = SDL_SCANCODE_F1;
constexpr SDL_Scancode DRAW_SPLINES = SDL_SCANCODE_F2;
constexpr SDL_Scancode STEP_MODE    = SDL_SCANCODE_P;
constexpr SDL_Scancode NEXT_STEP    = SDL_SCANCODE_N;
constexpr SDL_Scancode HOLD_TO_STEP = SDL_SCANCODE_M;
constexpr SDL_Scancode SPEED_UP     = SDL_SCANCODE_B;
constexpr SDL_Scancode SPEED_DOWN   = SDL_SCANCODE_V;
constexpr SDL_Scancode QUIT         = SDL_SCANCODE_ESCAPE;
};  // namespace Keybinds

struct GameConfig {
    const u32 window_flags     = SDL_WINDOW_BORDERLESS | SDL_WINDOW_OPENGL;
    glm::ivec2 window_position = { 0, 0 };

    const u32 fps         = 60;
    const u32 frame_delay = 1000 / fps;

    float speed               = 1.0f;
    bool step_mode            = false;
    bool use_const_delta_time = true;

    float hit_screen_shake_intensity = 5.0f;
    float hit_screen_shake_duration  = 30.0f;
    float hit_screen_shake_speed     = 0.5f;
};

class Game {
  public:
    bool is_running = false;

    void init();
    void run();

  private:
    u32 frame_start, last_frame_start;

    GameConfig game_config;

    SDL_Window* window;
    SDL_Renderer* sdl_renderer;
    SDL_GLContext gl_context;

    Renderer renderer;
    AudioManager audio_manager;

    MouseKeyboardInput mouse_keyboard_input;

    static const size_t NUM_PLAYERS = 2;

    Gamepad gamepads[NUM_PLAYERS];
    Player players[NUM_PLAYERS];

    Ball ball;

    Background background;

    Level level;
    LevelEditor level_editor;

    ConfigManager config_loader;

    uint score[2] = { 0, 0 };

#ifdef _DEBUG
    struct {
        bool collision_happened;
        VertexArray<DebugShader::Vertex> vao;
    } collision_point;
#endif

    enum GameMode { PLAY = 0, SPLINE_EDITOR = 1, LEVEL_EDITOR = 2 } game_mode;

    void simulate_world(float delta_time);
    void update_gui();
};