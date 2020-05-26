#pragma once
#include "pch.h"
#include "Game.h"
#include "DebugCallback.h"
#include "Systems.h"
#include "Shaders.h"
#include <imgui/imgui.h>
#include <imgui/imgui_impl_sdl.h>
#include <imgui/imgui_impl_opengl3.h>

U32 GamepadInput::num_gamepads = 0;

constexpr bool DEBUG_MODE = true;

void Game::init() {
    // Initialize SDL
    SDL_assert_always(SDL_Init(SDL_INIT_EVERYTHING) == 0);
    SDL_assert_always(IMG_Init(IMG_INIT_PNG) != 0);
    SDL_assert_always(TTF_Init() == 0);
    printf("SDL initialized\n");

    window = SDL_CreateWindow(
        "procAnim", SDL_WINDOWPOS_CENTERED, 0, render_data.window_size.x,
        render_data.window_size.y, game_config.window_flags);
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
    const char* glsl_version = "#version 330 core";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
                        SDL_GL_CONTEXT_PROFILE_CORE);

    if (DEBUG_MODE)
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);

    // Create openGL context
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

    // OpenGL configuration
    if (SDL_GL_SetSwapInterval(1) < 0) {
        printf("Warning: Unable to set VSync! SDL Error: %s\n", SDL_GetError());
    }
    glViewport(0, 0, render_data.window_size.x, render_data.window_size.y);
    // glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glPointSize(5.0f);

    if (DEBUG_MODE) {
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(handle_gl_debug_output, nullptr);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0,
                              nullptr, GL_TRUE);
    }

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Initialize member variables
    GLuint simple_shader_id = loadAndCompileShaderFromFile(
        "../src/shaders/simple.vert", "../src/shaders/simple.frag");
    GLuint rigged_shader_id = loadAndCompileShaderFromFile(
        "../src/shaders/rigged.vert", "../src/shaders/rigged.frag");
    GLuint debug_shader_id = loadAndCompileShaderFromFile(
        "../src/shaders/debug.vert", "../src/shaders/debug.frag");

    render_data.init(simple_shader_id, rigged_shader_id, debug_shader_id);

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
    player.tex = Texture::load_from_file("../assets/playerTexture.png");
    player.rigged_mesh.load_from_file("../assets/guy.fbx");
    player.gamepad_input = &gamepad_inputs[0];

    running = true;
};

bool Game::run() {
    while (running) {
        last_frame_start = frame_start;
        frame_start = SDL_GetTicks();

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT)
                running = false;
            if (event.type == SDL_WINDOWEVENT &&
                event.window.event == SDL_WINDOWEVENT_CLOSE &&
                event.window.windowID == SDL_GetWindowID(window))
                running = false;
        }

        poll_inputs(mouse_keyboard_input, gamepad_inputs);

        // Handle general keyboard inputs
        if (mouse_keyboard_input.key_down[SDL_SCANCODE_F1]) {
            render_data.draw_wireframes = !render_data.draw_wireframes;
        }
        if (mouse_keyboard_input.key_down[SDL_SCANCODE_F2]) {
            render_data.draw_bones = !render_data.draw_bones;
        }
        if (mouse_keyboard_input.key_down[SDL_SCANCODE_P]) {
            game_config.step_mode = !game_config.step_mode;
        }
        if (mouse_keyboard_input.key_down[SDL_SCANCODE_COMMA]) {
            game_config.speed = std::max(game_config.speed - 0.1f, 0.0f);
        }
        if (mouse_keyboard_input.key_down[SDL_SCANCODE_PERIOD]) {
            game_config.speed = game_config.speed + 0.1f;
        }
        if (mouse_keyboard_input.key_down[SDL_SCANCODE_ESCAPE]) {
            running = false;
        }

        float last_frame_duration =
            static_cast<float>(frame_start - last_frame_start);
        float frame_delay = static_cast<float>(game_config.frame_delay);

        if (!game_config.step_mode) {
            float delta_time =
                last_frame_duration / frame_delay * game_config.speed;
            update_player(delta_time, player, mouse_keyboard_input,
                          render_data);
        } else if (mouse_keyboard_input.key_down[SDL_SCANCODE_N] ||
                   mouse_keyboard_input.key[SDL_SCANCODE_M]) {
            update_player(game_config.speed, player, mouse_keyboard_input,
                          render_data);
        }

        update_gui(window, render_data, game_config, player);

        render(window, render_data, player);

        // Check for errors and clear error queue
        while (GLenum error = glGetError()) {
            printf("[OpenGL Error] %d\n", error);
            SDL_assert(!error);
        }

        // Wait for next frame
        U32 last_frame_time = SDL_GetTicks() - frame_start;
        if (game_config.frame_delay > last_frame_time)
            SDL_Delay(game_config.frame_delay - last_frame_time);
    }

    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
    return 0;
};

void RenderData::init(GLuint simple_shader_id, GLuint rigged_shader_id,
                      GLuint debug_shader_id) {
    SDL_assert_always(simple_shader_id != -1 && rigged_shader_id != -1);
    simple_shader.id = simple_shader_id;
    rigged_shader.id = rigged_shader_id;
    debug_shader.id = debug_shader_id;

    debug_shader.color_loc = glGetUniformLocation(debug_shader_id, "color");

#ifndef CPU_RENDERING
    rigged_shader.model_matrix_loc =
        glGetUniformLocation(rigged_shader_id, "model");
    rigged_shader.projection_matrix_loc =
        glGetUniformLocation(rigged_shader_id, "projection");
    rigged_shader.bonesLoc = glGetUniformLocation(rigged_shader_id, "bones");
#endif
}