#pragma once
#include "Game.h"
#include "DebugCallback.h"
#include "rendering/Shaders.h"
#include "CollisionDetection.h"
#include <sdl/SDL.h>
#include <sdl/SDL_image.h>
#include <imgui/imgui.h>
#include <imgui/imgui_impl_opengl3.h>
#include <imgui/imgui_impl_sdl.h>
#include <glm/gtx/matrix_transform_2d.hpp>

void Game::init() {
    // Initialize SDL
    SDL_assert_always(SDL_Init(SDL_INIT_EVERYTHING) == 0);
    SDL_assert_always(IMG_Init(IMG_INIT_PNG) != 0);

    config_loader.init(game_config, renderer);
    config_loader.load_config("../assets/config.ini");

    glm::ivec2 window_size = static_cast<glm::ivec2>(renderer.window_size());

    window = SDL_CreateWindow("procAnim", game_config.window_position.x,
                              game_config.window_position.y, window_size.x,
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
        SDL_TriggerBreakpoint();
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

    // Initialize SDL_mixer
    if (Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, 2, 4096) ==
        -1) {
        printf("Error initializing SDL_Mixer: %s", Mix_GetError());
        SDL_TriggerBreakpoint();
    }

    // Initialize member variables
    renderer.init();
    audio_manager.load_sounds();

#ifdef _DEBUG
    {
        GLuint index = 0;
        collision_point.vao.init(&index, 1, nullptr, 1, GL_DYNAMIC_DRAW);
    }
#endif

    mouse_keyboard_input.init(&renderer);

    gamepads[0].init(0);
    // gamepads[1].init(1);

    background.init("../assets/background.png");

    // Level
    level.load_from_file("../assets/default.level");
    level_editor.init(&level);

    // Player
    glm::vec3 position = {960.0f, 271.0f, 0.0f};
    players[0].init(position, glm::vec3(100.0f, 100.0f, 1.0f),
                    "../assets/playerTexture.png", "../assets/guy.fbx",
                    &gamepads[0], level.colliders());

    position.x += 50.0f;
    players[1].init(position, glm::vec3(100.0f, 100.0f, 1.0f),
                    "../assets/playerTexture.png", "../assets/guy.fbx",
                    &gamepads[1], level.colliders());

    // Ball
    ball.init(renderer.camera_center(), "../assets/ball.png");

    frame_start = SDL_GetTicks();
    is_running = true;
};

void Game::run() {
    last_frame_start = frame_start;
    frame_start = SDL_GetTicks();

    SDL_PumpEvents();

    // Get inputs
    mouse_keyboard_input.update();

    for (auto& pad : gamepads) {
        pad.update();
    }

    { // Process events
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
    }

    // Handle general keyboard inputs
    if (mouse_keyboard_input.key_down(Keybinds::DRAW_BONES)) {
        renderer.draw_bones = !renderer.draw_bones;
    }
    if (mouse_keyboard_input.key_down(Keybinds::DRAW_SPLINES)) {
        renderer.draw_leg_splines = !renderer.draw_leg_splines;
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

    // Move/zoom camera
    if (mouse_keyboard_input.mouse_button(MouseButton::MIDDLE)) {
        Vector mouse_move =
            static_cast<glm::vec2>(mouse_keyboard_input.mouse_move_screen());
        mouse_move.x *= -1.0f;

        renderer.camera_center_ += mouse_move;
    }

    {
        int scroll = mouse_keyboard_input.mouse_wheel_scroll;
        if (scroll != 0) {
            if (scroll > 0) {
                renderer.zoom_factor_ += static_cast<float>(scroll) / 5.0f;
            } else {
                renderer.zoom_factor_ += static_cast<float>(scroll) / 10.0f;
            }
        }
    }

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame(window);
    ImGui::NewFrame();

    update_gui();

    float last_frame_duration =
        static_cast<float>(frame_start - last_frame_start);
    float frame_delay = static_cast<float>(game_config.frame_delay);

    // Update components based on the current game_mode
    if (game_mode == PLAY) {

        float delta_time;
        if (game_config.use_const_delta_time || game_config.step_mode) {
            delta_time = game_config.speed;
        } else {
            delta_time = last_frame_duration / frame_delay * game_config.speed;
        }

        if (!game_config.step_mode ||
            (game_config.step_mode &&
             (mouse_keyboard_input.key_down(Keybinds::NEXT_STEP) ||
              mouse_keyboard_input.key(Keybinds::HOLD_TO_STEP)))) {
            simulate_world(delta_time);
        }

    } else if (game_mode == SPLINE_EDITOR) {
        if (!players[0].animator.spline_editor->update(mouse_keyboard_input)) {
            game_mode = PLAY;
        }
    } else if (game_mode == LEVEL_EDITOR) {
        if (!level_editor.update(renderer, mouse_keyboard_input)) {
            game_mode = PLAY;
        }
    }

    //              Render              //
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    background.render(renderer, renderer.camera_center());

    level.render(renderer);

    // Ball
    ball.render(renderer);

    // Players
    std::array<glm::mat3, RiggedShader::NUMBER_OF_BONES>
        bone_transforms[NUM_PLAYERS];
    if (renderer.draw_limbs || renderer.draw_wireframes) {
        for (size_t n_player = 0; n_player < NUM_PLAYERS; ++n_player) {
            const auto& player = players[n_player];

            SDL_assert(player.rigged_mesh.bones.size() <=
                       RiggedShader::NUMBER_OF_BONES);

            for (size_t n_bone = 0; n_bone < player.rigged_mesh.bones.size();
                 ++n_bone) {
                bone_transforms[n_player][n_bone] =
                    player.rigged_mesh.bones[n_bone].transform();
            }
        }
    }

    if (renderer.draw_limbs) {
        renderer.rigged_shader.use();
        for (size_t n_player = 0; n_player < NUM_PLAYERS; ++n_player) {
            const auto& player = players[n_player];

            renderer.rigged_shader.set_model(&player.model);
            renderer.rigged_shader.set_bone_transforms(
                bone_transforms[n_player].data());
            renderer.rigged_shader.set_texture(player.texture);

            player.rigged_mesh.vao.draw(GL_TRIANGLES);
        }
    }

    if (renderer.draw_body) {
        renderer.textured_shader.use();
        for (const auto& player : players) {
            renderer.textured_shader.set_model(&player.model);
            renderer.textured_shader.set_texture(player.texture);

            player.body_mesh.vao.draw(GL_TRIANGLES);
        }
    }

    if (renderer.draw_wireframes) {
        renderer.rigged_debug_shader.use();
        for (size_t n_player = 0; n_player < NUM_PLAYERS; ++n_player) {
            const auto& player = players[n_player];

            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            // NEXT: Why does this not display anything?
            renderer.rigged_debug_shader.set_model(&player.model);
            renderer.rigged_debug_shader.set_color(Color::BLUE);
            renderer.rigged_debug_shader.set_bone_transforms(
                bone_transforms[n_player].data());

            player.rigged_mesh.vao.draw(GL_TRIANGLES);
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }
    }

    if (renderer.draw_bones) {
        renderer.bone_shader.use();
        for (size_t n_player = 0; n_player < NUM_PLAYERS; ++n_player) {
            const auto& player = players[n_player];
            renderer.bone_shader.set_model(&player.model);
            renderer.bone_shader.set_color(Color::RED);
            renderer.bone_shader.set_bone_transforms(
                bone_transforms[n_player].data());

            glLineWidth(2.0f);
            player.rigged_mesh.bones_vao.draw(GL_LINES);
            glPointSize(1.0f);
            player.rigged_mesh.bones_vao.draw(GL_POINTS);
        }
    }

    if (renderer.draw_colliders) {
        renderer.debug_shader.use();
        for (const auto& player : players) {
            renderer.debug_shader.set_color(Color::ORANGE);

            const auto collider = player.body_collider();
            glm::mat3 model = glm::translate(glm::mat3(1.0f), collider.center);
            model = glm::scale(model, glm::vec2(collider.radius));

            renderer.debug_shader.set_model(&model);
            renderer.debug_shader.CIRCLE_VAO.draw(GL_LINE_LOOP);
        }
    }

    if (renderer.draw_leg_splines) {
        renderer.debug_shader.use();
        renderer.debug_shader.set_color(Color::GREEN);

        // All the spline positions are already in world space, so set the model
        // matrix to unity
        glm::mat3 model(1.0f);
        renderer.debug_shader.set_model(&model);

        for (const auto& player : players) {
            player.animator.limbs[Animator::LEFT_LEG].spline.render(renderer,
                                                                    true);
            player.animator.limbs[Animator::RIGHT_LEG].spline.render(renderer,
                                                                     true);
        }
    }

#ifdef _DEBUG
    // Debug data
    if (collision_point.collision_happened) {
        renderer.debug_shader.set_color(Color::LIGHT_BLUE);
        glPointSize(2.0f);
        collision_point.vao.draw(GL_POINTS);
    }
#endif

    if (game_mode == SPLINE_EDITOR) {
        players[0].animator.spline_editor->render(renderer, true);
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

void Game::simulate_world(float delta_time) {

    for (auto& player : players) {
        if (player.freeze_duration > 0.0f) {
            player.freeze_duration -= delta_time;
            continue;
        }

        player.update(delta_time, level.colliders());

        //              Resolve collisions              //
        Point new_player_position;
        Vector new_player_velocity = player.velocity;

        // Body collisions with level
        if (player.state == Player::HITSTUN) {
            BallisticMoveResult result = get_ballistic_move_result(
                player.body_collider(), player.velocity, delta_time,
                level.colliders());

            new_player_position = result.new_position;
            new_player_velocity = result.new_velocity;

            if (result.last_hit_diretcion != Direction::NONE) {
                audio_manager.play(Sound::WALL_BOUNCE);
            }

        } else { // player.state != Player::HITSTUN
            Vector player_move = player.velocity * delta_time;
            CollisionData first_collision = find_first_collision_moving_circle(
                player.body_collider(), player_move, level.colliders());

            if (first_collision.direction == Direction::NONE) {
                new_player_position = player.position() + player_move;

            } else {
                Vector remaining_player_move;
                if (first_collision.direction == Direction::DOWN ||
                    first_collision.direction == Direction::UP) {

                    new_player_velocity.y = 0.0f;

                    remaining_player_move = Vector(
                        player_move.x * (1.0f - first_collision.t), 0.0f);

                } else { // collision.direction == Direction::LEFT ||
                         // Direction::RIGHT
                    new_player_velocity.x = 0.0f;

                    remaining_player_move = Vector(
                        0.0f, player_move.y - (1.0f - first_collision.t));
                }

                Circle body_collider = player.body_collider();
                body_collider.center += player_move * first_collision.t;

                CollisionData second_collision =
                    find_first_collision_moving_circle(body_collider,
                                                       remaining_player_move,
                                                       level.colliders());

                SDL_assert(second_collision.direction !=
                           first_collision.direction);

                if (second_collision.direction != Direction::NONE) {
                    // The player hit a corner, can't move any further
                    new_player_velocity = Vector(0.0f);
                }

                new_player_position =
                    body_collider.center +
                    remaining_player_move * second_collision.t;
            }
        }

        if (player.state != Player::HITSTUN) {
            // Keep player above ground, check grounded status
            auto ground_under_player =
                level.find_ground_under(new_player_position);
            if (!ground_under_player ||
                new_player_position.y - ground_under_player->max(1) >
                    Player::GROUND_HOVER_DISTANCE +
                        2.0f /* small tolerance */) {

                player.grounded = false;
                player.state = Player::FALLING;

            } else {
                new_player_velocity.y = std::max(new_player_velocity.y, 0.0f);
                new_player_position.y =
                    ground_under_player->max(1) + Player::GROUND_HOVER_DISTANCE;

                player.grounded = true;
                player.can_double_jump = true;
                if (player.state == Player::FALLING) {
                    player.state = Player::STANDING;
                }
            }
        }

        if (!player.grounded) {
            new_player_velocity.y -= Player::GRAVITY;
        }

        player.velocity = new_player_velocity;
        player.position_ = new_player_position;
    } // End for each player

    { // Player/Player Collisions // TODO
        Circle colliders[2];
        colliders[0] = players[0].body_collider();
        colliders[1] = players[1].body_collider();

        glm::vec2 between_colliders = colliders[0].center - colliders[1].center;
        float distance = glm::length(between_colliders);

        if (distance < colliders[0].radius + colliders[1].radius) {
            // players[0].velocity +=
        }
    }

    // Collisions between weapon and weapon/ball
    const Segment* weapons[NUM_PLAYERS];
    for (size_t i = 0; i < NUM_PLAYERS; ++i) {
        weapons[i] = &players[i].weapon_collider;
    }

    if (weapons[0]->line() != glm::vec2(0.0f) ||
        weapons[1]->line() != glm::vec2(0.0f)) {

        // Weapon vs. weapon
        float t;
        Point collision_pos;
        if (intersect_segment_segment(*weapons[0], *weapons[1], &t,
                                      &collision_pos)) {
            printf("Weapons colliding at t=%.2f, pos: %.2f, %.2f\n", t,
                   collision_pos.x, collision_pos.y);

#ifdef _DEBUG
            std::array<DebugShader::Vertex, 1> shader_vertex = {collision_pos};
            collision_point.vao.update_vertex_data(shader_vertex);
            collision_point.collision_happened = true;
        } else {
            collision_point.collision_happened = false;
#endif
        }

        // Weapon vs. ball
        for (size_t i = 0; i < NUM_PLAYERS; ++i) {
            auto& weapon = weapons[i];
            auto& player = players[i];
            if (player.time_since_last_hit >= Player::HIT_COOLDOWN &&
                glm::length(weapon->line()) > player.body_collider().radius &&
                intersect_segment_circle(*weapon, ball.collider())) {

                // The ball was hit
                vec2 hit_direction = player.weapon_collider.line() -
                                     player.last_weapon_collider.line();

                ball.set_velocity(hit_direction * Player::HIT_SPEED_MULTIPLIER);

                player.time_since_last_hit = 0.0f;

                printf("Player %zd hit with %f, %f\n", i, hit_direction.x,
                       hit_direction.y);

                const float screen_shake_duration =
                    game_config.hit_screen_shake_duration *
                    glm::length(hit_direction);

                player.freeze_duration = screen_shake_duration;
                ball.freeze_duration = screen_shake_duration;
                renderer.shake_screen(game_config.hit_screen_shake_intensity *
                                          glm::length(hit_direction),
                                      screen_shake_duration,
                                      game_config.hit_screen_shake_speed);
                audio_manager.play(Sound::HIT_2);
            }
        }
    }

    ball.update(delta_time, level.colliders());
    renderer.update(delta_time);
}

void Game::update_gui() {
    using namespace ImGui;
    //////          Debug controls window           //////
    Begin("Debug control", NULL, ImGuiWindowFlags_NoTitleBar);

    Text("Mode");
    RadioButton("Play", (int*)&game_mode, GameMode::PLAY);
    RadioButton("Spline editor", (int*)&game_mode, GameMode::SPLINE_EDITOR);
    RadioButton("Level editor", (int*)&game_mode, GameMode::LEVEL_EDITOR);

    // Config editor
    NewLine();
    static bool show_config_window = true;
    Checkbox("Show config editor", &show_config_window);

    if (show_config_window) {
        show_config_window = config_loader.display_ui_window();
    }

    Separator();
    Text("Display debug windows");
    { // Player windows
        static bool player_window_open[2] = {
            true, true}; // NOTE: This is not saved in config file
        for (size_t i = 0; i < NUM_PLAYERS; ++i) {
            char label[10];
            sprintf_s(label, "Player %zd", i);
            Checkbox(label, &player_window_open[i]);
            if (player_window_open[i]) {
                player_window_open[i] = players[i].display_debug_ui(i);
            }
        }
    }
    {
        static bool ball_window_open = true;
        Checkbox("Ball", &ball_window_open);
        if (ball_window_open) {
            ball.display_debug_ui();
        }
    }

    Separator();
    Text("Animation controls");
    PushItemWidth(100);
    DragFloat("Step distance multiplier",
              &players[0].animator.step_distance_multiplier, 1.0f, 0.0f, 0.0f,
              "%.1f");
    DragFloat2("Interpolation speed min/max",
               &players[0].animator.interpolation_speed_multiplier.min, 0.01f);
    PopItemWidth();

    End();
}