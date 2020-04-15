#pragma once
#include "pch.h"
#include "Game.h"
#include "DebugCallback.h"
#include "Systems.h"
#include "Shaders.h"

U32 GamepadInput::num_gamepads = 0;

constexpr bool DEBUG_MODE = true;

void Game::init() {
    // Initialize SDL
    SDL_assert_always(SDL_Init(SDL_INIT_EVERYTHING) == 0);
    SDL_assert_always(IMG_Init(IMG_INIT_PNG) != 0);
    SDL_assert_always(TTF_Init() == 0);
    printf("SDL initialized\n");

    window = SDL_CreateWindow(
        "procAnim", SDL_WINDOWPOS_CENTERED, 0, game_config.window_size.x,
        game_config.window_size.y, game_config.window_flags);
    SDL_assert_always(window);
    printf("Window created\n");

    renderer = SDL_CreateRenderer(window, -1, 0);

    const int numJoysticks = SDL_NumJoysticks();
    for (int i = 0; i < numJoysticks; ++i) {
        if (!SDL_IsGameController(i)) {
            printf("[Input] Joystick%d is not a supported GameController!\n",
                   i);
            continue;
        }
        ++GamepadInput::num_gamepads;
    }

    // Use OpenGL 3.3 core
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
                        SDL_GL_CONTEXT_PROFILE_CORE);

    if (DEBUG_MODE)
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);

    // Create context
    gl_context = SDL_GL_CreateContext(window);
    if (gl_context == NULL) {
        printf("OpenGL context could not be created! SDL Error: %s\n",
               SDL_GetError());
        SDL_assert(false);
    }
    // Initialize GLEW
    glewExperimental = GL_TRUE;
    GLenum glewError = glewInit();
    if (glewError != GLEW_OK) {
        printf("Error initializing GLEW! %s\n", glewGetErrorString(glewError));
    }

    // Use Vsync
    if (SDL_GL_SetSwapInterval(1) < 0) {
        printf("Warning: Unable to set VSync! SDL Error: %s\n", SDL_GetError());
    }

    // OpenGL configuration
    glViewport(0, 0, game_config.window_size.x, game_config.window_size.y);
    // glEnable(GL_CULL_FACE);
    // glEnable(GL_BLEND);
    // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    if (DEBUG_MODE) {
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(handle_gl_debug_output, nullptr);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0,
                              nullptr, GL_TRUE);
    }

    // Initialize members
    GLuint simple_shader_id = loadAndCompileShaderFromFile(
        "../src/shaders/simple.vert", "../src/shaders/simple.frag");
    GLuint rigged_shader_id =
#ifdef CPU_RENDERING
        loadAndCompileShaderFromFile("../src/shaders/debug.vert",
                                     "../src/shaders/rigged.frag");
#else
        loadAndCompileShaderFromFile("../src/shaders/rigged.vert",
                                     "../src/shaders/rigged.frag");
#endif

    render_data.init(simple_shader_id, rigged_shader_id);

    mouse_keyboard_input.init();

    // Open gamepads
    for (U32 i = 0; i < GamepadInput::num_gamepads; ++i) {
        gamepad_inputs[i].sdl_ptr = SDL_GameControllerOpen(i);
        if (!gamepad_inputs[i].sdl_ptr) {
            printf("[Input] Error opening gamepad%I32d: %s\n", i,
                   SDL_GetError());
        }
    }

    // Player
    player.pos = {1920.0f / 2.0f, 1080.0f / 2.0f};
    player.tex = Texture::load_from_file("../assets/red100x100.png");
    player.rigged_mesh = RiggedMesh::load_from_file("../assets/guy.dae");
    player.gamepad_input = &gamepad_inputs[0];

    // Initialize other structs
    Mesh::init();

    running = true;
};

bool Game::run() {
    while (running) {
        frameStart = SDL_GetTicks();

        poll_inputs(mouse_keyboard_input, gamepad_inputs);

        update_player(player);

        render(window, render_data, player);

        // Check for errors and clear error queue
        while (GLenum error = glGetError()) {
            printf("[OpenGL Error] %d\n", error);
            SDL_assert(!error);
        }

        // Wait for next frame
        U32 frameTime = SDL_GetTicks() - frameStart;
        if (game_config.frame_delay > frameTime)
            SDL_Delay(game_config.frame_delay - frameTime);
    }

    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
    return 0;
};

void RenderData::init(GLuint simple_shader_id, GLuint rigged_shader_id) {
    SDL_assert_always(simple_shader_id != -1 && rigged_shader_id != -1);
    simple_shader.id = simple_shader_id;
    rigged_shader.id = rigged_shader_id;

#ifndef CPU_RENDERING
    rigged_shader.model_matrix_loc =
        glGetUniformLocation(rigged_shader_id, "model");
    rigged_shader.projection_matrix_loc =
        glGetUniformLocation(rigged_shader_id, "projection");
    rigged_shader.bonesLoc = glGetUniformLocation(rigged_shader_id, "bones");
#endif
}