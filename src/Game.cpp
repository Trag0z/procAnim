#pragma once
#include "pch.h"
#include "Game.h"
#include "DebugCallback.h"
#include "Shaders.h"

void Game::init() {
    // Initialize SDL
    SDL_assert_always(SDL_Init(SDL_INIT_EVERYTHING) == 0);
    SDL_assert_always(IMG_Init(IMG_INIT_PNG) != 0);

    glm::ivec2 window_size = static_cast<glm::ivec2>(renderer.window_size());

    window = SDL_CreateWindow("procAnim", 3840, 956, window_size.x,
                              window_size.y, game_config.window_flags);
    SDL_assert_always(window);

    sdl_renderer = SDL_CreateRenderer(window, -1, 0);

    // Use OpenGL 3.3 core
    const char* glsl_version = "#version 330 core";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
                        SDL_GL_CONTEXT_PROFILE_CORE);

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
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glPointSize(5.0f);

#ifdef _DEBUG
    printf("DEBUG MODE\n");
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(handle_gl_debug_output, nullptr);
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr,
                          GL_TRUE);
#endif

    // Setup ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Initialize member variables
    renderer.init();

    mouse_keyboard_input.init(&renderer);

    gamepads = Gamepad::init();

    background.init("../assets/background.png");

    // Player
    glm::vec3 position = {1920.0f / 2.0f, 1080.0f / 2.0f, 0.0f};
    player.init(position, glm::vec3(100.0f, 100.0f, 1.0f),
                "../assets/playerTexture.png", "../assets/guy.fbx",
                &gamepads[0]);

    // Level
    level.load_from_file("../assets/default.level");
    level_editor.init(&level);
    BoxCollider::TEXTURE.load_from_file("../assets/ground.png");

    frame_start = SDL_GetTicks();
    is_running = true;
};

void Game::run() {
    last_frame_start = frame_start;
    frame_start = SDL_GetTicks();

    SDL_PumpEvents();

    mouse_keyboard_input.update();
    for (auto& pad : gamepads) {
        pad.update();
    }

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        ImGui_ImplSDL2_ProcessEvent(&event);
        if (event.type == SDL_QUIT ||
            (event.type == SDL_WINDOWEVENT &&
             event.window.event == SDL_WINDOWEVENT_CLOSE &&
             event.window.windowID == SDL_GetWindowID(window))) {
            is_running = false;
        } else if (event.type == SDL_MOUSEWHEEL) {
            mouse_keyboard_input.mouse_wheel_scroll = event.wheel.y;
        }
    }

    // Handle general keyboard inputs
    if (mouse_keyboard_input.key_down(Keybinds::DRAW_WIREFRAEMS)) {
        renderer.draw_wireframes = !renderer.draw_wireframes;
    }
    if (mouse_keyboard_input.key_down(Keybinds::DRAW_BONES)) {
        renderer.draw_bones = !renderer.draw_bones;
    }
    if (mouse_keyboard_input.key_down(Keybinds::DRAW_WALK_SPLINES)) {
        renderer.draw_walk_splines = !renderer.draw_walk_splines;
    }
    if (mouse_keyboard_input.key_down(Keybinds::STEP_MODE)) {
        game_config.step_mode = !game_config.step_mode;
    }
    if (mouse_keyboard_input.key_down(Keybinds::SPEED_UP)) {
        game_config.speed *= 0.5f;
    }
    if (mouse_keyboard_input.key_down(Keybinds::SPEED_DOWN)) {
        game_config.speed *= 2.0f;
    }
    if (mouse_keyboard_input.key_down(Keybinds::QUIT)) {
        if (game_mode == PLAY) {
            is_running = false;
        } else if (game_mode == SPLINE_EDITOR || game_mode == LEVEL_EDITOR) {
            game_mode = PLAY;
        }
    }

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame(window);
    ImGui::NewFrame();

    update_gui();

    float last_frame_duration =
        static_cast<float>(frame_start - last_frame_start);
    float frame_delay = static_cast<float>(game_config.frame_delay);

    // Update stuff based on the current game_mode
    if (game_mode == PLAY) {
        if (!game_config.step_mode) {
            float delta_time;
            if (game_config.use_const_delta_time) {
                delta_time = game_config.speed;
            } else {
                delta_time =
                    last_frame_duration / frame_delay * game_config.speed;
            }

            player.update(delta_time, level.colliders(), mouse_keyboard_input);

        } else if (mouse_keyboard_input.key_down(SDL_SCANCODE_N) ||
                   mouse_keyboard_input.key(SDL_SCANCODE_M)) {
            player.update(game_config.speed, level.colliders(),
                          mouse_keyboard_input);
        }
    } else if (game_mode == SPLINE_EDITOR) {
        if (!player.animator.spline_editor->update(mouse_keyboard_input)) {
            game_mode = PLAY;
        }
    } else if (game_mode == LEVEL_EDITOR) {
        if (!level_editor.update(renderer, mouse_keyboard_input)) {
            game_mode = PLAY;
        }
    }

    // Render
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    renderer.update_camera(player.get_position() + glm::vec2(0.0f, 200.0f));

    background.render(renderer, renderer.camera_center());

    level.render(renderer);

    player.render(renderer);

    if (game_mode == SPLINE_EDITOR) {
        player.animator.spline_editor->render(renderer, true);
    } else if (game_mode == LEVEL_EDITOR) {
        level_editor.render(renderer);
    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    SDL_GL_SwapWindow(window);

    // Wait for next frame
    u32 last_frame_time = SDL_GetTicks() - frame_start;
    if (game_config.frame_delay > last_frame_time) {
        SDL_Delay(game_config.frame_delay - last_frame_time);
    }
}

void Game::update_gui() {
    using namespace ImGui;

    //////          Debug controls window           //////
    Begin("Debug control", NULL, ImGuiWindowFlags_NoTitleBar);
    Checkbox("Render player model", &renderer.draw_models);
    Checkbox("Render wireframes", &renderer.draw_wireframes);
    Checkbox("Render bones", &renderer.draw_bones);
    Checkbox("Render walk splines", &renderer.draw_walk_splines);

    NewLine();
    Checkbox("Use constant delta time", &game_config.use_const_delta_time);
    SetNextItemWidth(100);
    DragFloat("Game speed", &game_config.speed, 0.1f, 0.0f, 100.0f, "%.2f");

    NewLine();
    Checkbox("Arm follows mouse", &player.animator.arm_follows_mouse);
    Checkbox("Step mode", &game_config.step_mode);

    NewLine();
    Text("Mode");
    RadioButton("Play", (int*)&game_mode, GameMode::PLAY);
    RadioButton("Spline editor", (int*)&game_mode, GameMode::SPLINE_EDITOR);
    RadioButton("Level editor", (int*)&game_mode, GameMode::LEVEL_EDITOR);

    End();

    //////          Limb data display window            //////
    Begin("Limb data", NULL);

    char label[128];

    sprintf_s(label, "% 6.1f, % 6.1f", player.position.x, player.position.y);
    Text("Player position: ");
    SameLine();
    bool changed_value =
        DragFloat2("Player position", value_ptr(player.position), 1.0f, 0.0f,
                   0.0f, "% .2f");
    if (changed_value) {
        player.grounded = false;
    }

    Text("Target Positions");
    Columns(4);
    Separator();
    Text("Arm_L");
    NextColumn();
    Text("Arm_R");
    NextColumn();
    Text("Leg_L");
    NextColumn();
    Text("Leg_R");
    NextColumn();
    Separator();

    for (const auto& limb : player.animator.limbs) {
        glm::vec2 target_world_pos =
            player.local_to_world_space(limb.spline.get_point(Spline::P2));
        sprintf_s(label, "%6.1f, %6.1f", target_world_pos.x,
                  target_world_pos.y);
        Text(label);
        NextColumn();
    }

    Columns(1);
    Separator();

    NewLine();
    Text("Limb data");
    Columns(4);
    Separator();
    Text("Name");
    NextColumn();
    Text("Rotation deg/rad");
    NextColumn();
    Text("Head Position");
    NextColumn();
    Text("Tail Position");
    NextColumn();
    Separator();

    for (const auto& bone : player.rigged_mesh.bones) {
        Text(bone.name.c_str());
        NextColumn();

        sprintf_s(label, "% 6.1f /% 1.2f", radToDeg(bone.rotation),
                  bone.rotation);
        Text(label);
        NextColumn();

        glm::vec2 head_world_pos = player.local_to_world_space(
            bone.get_transform() * bone.bind_pose_transform *
            glm::vec3(bone.head(), 1.0f));
        sprintf_s(label, "% 7.1f, % 7.1f", head_world_pos.x, head_world_pos.y);
        Text(label);
        NextColumn();

        glm::vec2 tail_world_pos = player.local_to_world_space(
            glm::vec2(bone.get_transform() * bone.bind_pose_transform *
                      glm::vec3(bone.tail, 1.0f)));
        sprintf_s(label, "% 7.1f, % 7.1f", tail_world_pos.x, tail_world_pos.y);
        Text(label);
        NextColumn();
    }
    Columns(1);
    Separator();

    End();
}