#pragma once
#include "pch.h"
#include "Game.h"
#include "DebugCallback.h"
#include "Systems.h"
#include "Shaders.h"

constexpr bool DEBUG_MODE = true;

void Game::init() {
    // Initialize SDL
    SDL_assert_always(SDL_Init(SDL_INIT_EVERYTHING) == 0);
    SDL_assert_always(IMG_Init(IMG_INIT_PNG) != 0);
    SDL_assert_always(TTF_Init() == 0);

    glm::ivec2 window_size = static_cast<glm::ivec2>(renderer.window_size());

    window =
        SDL_CreateWindow("procAnim", SDL_WINDOWPOS_CENTERED, 0, window_size.x,
                         window_size.y, game_config.window_flags);
    SDL_assert_always(window);

    sdl_renderer = SDL_CreateRenderer(window, -1, 0);

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
    glViewport(0, 0, window_size.x, window_size.y);
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

    // Setup ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Initialize member variables
    renderer.init();

    mouse_keyboard_input.init(window_size.y);

    gamepads = Gamepad::init();

    background.init("../assets/background.png");

    // Player
    glm::vec3 position = {1920.0f / 2.0f, 1080.0f / 2.0f, 0.0f};
    player.init(position, glm::vec3(100.0f, 100.0f, 1.0f),
                "../assets/playerTexture.png", "../assets/guy.fbx",
                &gamepads[0]);

    // Ground
    ground =
        BoxCollider({position.x, position.y - 400.0f}, {1920.0f / 2.1f, 10.0f});

    frame_start = SDL_GetTicks();
    running = true;
};

bool Game::run() {
    while (running) {
        last_frame_start = frame_start;
        frame_start = SDL_GetTicks();

        SDL_PumpEvents();
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

        mouse_keyboard_input.update();
        for (auto& pad : gamepads) {
            pad.update();
        }

        // Handle general keyboard inputs
        if (mouse_keyboard_input.key_down(SDL_SCANCODE_F1)) {
            renderer.draw_wireframes = !renderer.draw_wireframes;
        }
        if (mouse_keyboard_input.key_down(SDL_SCANCODE_F2)) {
            renderer.draw_bones = !renderer.draw_bones;
        }
        if (mouse_keyboard_input.key_down(SDL_SCANCODE_P)) {
            game_config.step_mode = !game_config.step_mode;
        }
        if (mouse_keyboard_input.key_down(SDL_SCANCODE_COMMA)) {
            game_config.speed *= 0.5f;
        }
        if (mouse_keyboard_input.key_down(SDL_SCANCODE_PERIOD)) {
            game_config.speed *= 2.0f;
        }
        if (mouse_keyboard_input.key_down(SDL_SCANCODE_ESCAPE)) {
            running = false;
        }

        update_gui(window, renderer, game_config, player);

        float last_frame_duration =
            static_cast<float>(frame_start - last_frame_start);
        float frame_delay = static_cast<float>(game_config.frame_delay);

        if (!game_config.step_mode) {
            float delta_time;
            if (game_config.use_const_delta_time) {
                delta_time = game_config.speed;
            } else {
                delta_time =
                    last_frame_duration / frame_delay * game_config.speed;
            }

            player.update(delta_time, ground, mouse_keyboard_input);
        } else if (mouse_keyboard_input.key_down(SDL_SCANCODE_N) ||
                   mouse_keyboard_input.key(SDL_SCANCODE_M)) {
            player.update(game_config.speed, ground, mouse_keyboard_input);
        }

        renderer.render(window, background, player, ground);

        // Wait for next frame
        U32 last_frame_time = SDL_GetTicks() - frame_start;
        if (game_config.frame_delay > last_frame_time)
            SDL_Delay(game_config.frame_delay - last_frame_time);
    }

    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
    return 0;
}
