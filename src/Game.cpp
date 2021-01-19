#pragma once
#include "pch.h"
#include "Game.h"
#include "DebugCallback.h"
#include "rendering/Shaders.h"

void Game::init() {
    // Initialize SDL
    SDL_assert_always(SDL_Init(SDL_INIT_EVERYTHING) == 0);
    SDL_assert_always(IMG_Init(IMG_INIT_PNG) != 0);

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

    {
        GLuint index = 0;
        collision_point.vao.init(&index, 1, nullptr, 1, GL_DYNAMIC_DRAW);
    }

    mouse_keyboard_input.init(&renderer);

    gamepads[0].init(0);
    gamepads[1].init(1);

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

    renderer.update_camera(glm::vec2(1022.0f, 693.0f));

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

    // Players
    std::array<glm::mat3, RiggedShader::NUMBER_OF_BONES>
        bone_transforms[NUM_PLAYERS];
    if (renderer.draw_limbs || renderer.draw_wireframe) {
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

    if (renderer.draw_wireframe) {
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
            glm::mat3 model =
                glm::translate(glm::mat3(1.0f), collider.position);
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

    // Debug data
    if (collision_point.collision_happened) {
        renderer.debug_shader.set_color(Color::LIGHT_BLUE);
        glPointSize(2.0f);
        collision_point.vao.draw(GL_POINTS);
    }

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

    for (size_t n_player = 0; n_player < NUM_PLAYERS; ++n_player) {
        auto& player = players[n_player];
        player.update(delta_time, level.colliders(), mouse_keyboard_input);

        //              Resolve collisions              //
        glm::vec2 new_player_position;
        glm::vec2 new_player_velocity = player.velocity;
        const glm::vec2 player_move = player.velocity * delta_time;

        { // Body collisions with level
            CircleCollider body_collider = player.body_collider();
            CollisionData first_collision = find_first_collision_sweep_prune(
                body_collider, player_move, level.colliders());

            if (first_collision.direction == Direction::NONE) {
                new_player_position = player.position() + player_move;
                // player.velocity stays the same
            } else {
                body_collider.position += first_collision.move_until_collision;

                if (player.state == Player::HITSTUN) {
                    audio_manager.play(Sound::WALL_BOUNCE);

                    // Finds the next collision, resolves it, sets
                    // new_player_velocity and and body_collider to new values
                    // and also sets player_grounded if the player hit the
                    // ground.
                    auto resolve_next_hitstun_collision =
                        [delta_time, &new_player_velocity,
                         &body_collider](bool& player_grounded,
                                         const CollisionData& last_collision,
                                         const std::list<BoxCollider> boxes)
                        -> CollisionData {
                        SDL_assert(last_collision.direction != NONE);

                        CollisionData next_collision;
                        if (last_collision.direction == LEFT ||
                            last_collision.direction == RIGHT) {

                            new_player_velocity.x *= -1.0f;

                            glm::vec2 remaining_player_move =
                                new_player_velocity *
                                (delta_time - last_collision.time);
                            next_collision = find_first_collision_sweep_prune(
                                body_collider, remaining_player_move, boxes);

                        } else if (last_collision.direction == UP) {
                            new_player_velocity.y *= -1.0f;

                            glm::vec2 remaining_player_move =
                                new_player_velocity *
                                (delta_time - last_collision.time);
                            next_collision = find_first_collision_sweep_prune(
                                body_collider, remaining_player_move, boxes);

                        } else { // last_collison.direction == DOWN
                            new_player_velocity = glm::vec2(0.0f);
                            next_collision.move_until_collision =
                                glm::vec2(0.0f);
                            next_collision.direction = NONE;
                            player_grounded = true;

                            SDL_assert(
                                find_first_collision_sweep_prune(
                                    body_collider, new_player_velocity, boxes)
                                    .direction == NONE);
                        }
                        return next_collision;
                    };

                    const size_t max_collision_iterations = 5;
                    size_t collision_iteration = 0;
                    CollisionData next_collision = first_collision;
                    glm::vec2 complete_player_move =
                        first_collision.move_until_collision;

                    do {
                        next_collision = resolve_next_hitstun_collision(
                            player.grounded, next_collision, level.colliders());
                        complete_player_move +=
                            next_collision.move_until_collision;
                    } while (next_collision.direction != NONE &&
                             ++collision_iteration < max_collision_iterations);

                    SDL_assert(collision_iteration < max_collision_iterations);

                    new_player_position =
                        player.position() +
                        complete_player_move *
                            0.95f; // TODO: Remove this margin?

                } else { // state != Player::HITSTUN
                    glm::vec2 remaining_player_move;
                    if (first_collision.direction == Direction::DOWN ||
                        first_collision.direction == Direction::UP) {

                        new_player_velocity.y = 0.0f;

                        remaining_player_move = glm::vec2(
                            player_move.x -
                                first_collision.move_until_collision.x,
                            0.0f);

                    } else { // collision.direction == LEFT || RIGHT
                        new_player_velocity.x = 0.0f;

                        remaining_player_move = glm::vec2(
                            0.0f, player_move.y -
                                      first_collision.move_until_collision.y);
                    }
                    CollisionData second_collision =
                        find_first_collision_sweep_prune(body_collider,
                                                         remaining_player_move,
                                                         level.colliders());
                    SDL_assert(second_collision.direction !=
                               first_collision.direction);

                    if (second_collision.direction != Direction::NONE) {
                        // The player hit a corner, can't move any further
                        new_player_velocity = glm::vec2(0.0f);
                    }

                    new_player_position = player.position() +
                                          first_collision.move_until_collision +
                                          second_collision.move_until_collision;
                }
            }
        }

        if (player.state != Player::HITSTUN) {
            // Keep player above ground, check grounded status
            auto ground_under_player =
                level.find_ground_under(new_player_position);
            if (!ground_under_player ||
                new_player_position.y - ground_under_player->top_edge() >
                    player.GROUND_HOVER_DISTANCE + 2.0f /* small tolerance */) {

                player.grounded = false;
                player.state = Player::FALLING;

            } else {
                new_player_velocity.y = std::max(new_player_velocity.y, 0.0f);
                new_player_position.y = ground_under_player->top_edge() +
                                        player.GROUND_HOVER_DISTANCE;

                player.grounded = true;
                if (player.state == Player::FALLING) {
                    player.state = Player::STANDING;
                }
            }
        }

        if (!player.grounded) {
            new_player_velocity.y -= game_config.gravity;
        }

        player.velocity = new_player_velocity;
        player.position_ = new_player_position;
    }

    { // Player/Player Collisions // TODO
        CircleCollider colliders[2];
        colliders[0] = players[0].body_collider();
        colliders[1] = players[1].body_collider();

        glm::vec2 between_colliders =
            colliders[0].position - colliders[1].position;
        float distance = glm::length(between_colliders);

        if (distance < colliders[0].radius + colliders[1].radius) {
            // players[0].velocity +=
        }
    }

    // Collisions between weapons and weapons/bodies
    const LineCollider* weapons[NUM_PLAYERS];
    for (size_t i = 0; i < NUM_PLAYERS; ++i) {
        weapons[i] = &players[i].weapon_collider;
    }

    if (weapons[0]->line != glm::vec2(0.0f) ||
        weapons[1]->line != glm::vec2(0.0f)) {

        // Weapon vs. weapon
        float t;
        if (weapons[0]->intersects(*weapons[1], &t)) {
            glm::vec2 collision_pos = weapons[0]->start + weapons[0]->line * t;
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

        // Weapon vs. body
        for (size_t i = 0; i < NUM_PLAYERS; ++i) {
            auto& weapon = weapons[i];
            auto& attacking_player = players[i];
            auto& hit_player = players[1 - i];
            if (attacking_player.time_since_last_hit >= Player::HIT_COOLDOWN &&
                glm::length(weapon->line) >
                    attacking_player.body_collider().radius &&
                weapon->intersects(hit_player.body_collider())) {

                // A player was hit
                glm::vec2 hit_direction =
                    attacking_player.weapon_collider.line -
                    attacking_player.last_weapon_collider.line;

                hit_player.state = Player::HITSTUN;
                hit_player.hitstun_duration =
                    glm::length(hit_direction) *
                    Player::HITSTUN_DURATION_MULTIPLIER;
                hit_player.grounded = false;

                hit_player.velocity =
                    hit_direction * players[i].HIT_SPEED_MULTIPLIER;

                attacking_player.time_since_last_hit = 0.0f;

                printf("Player %zd hit with %f, %f\n", i, hit_direction.x,
                       hit_direction.y);

                audio_manager.play(Sound::HIT_2);
            }
        }
    }
}

void Game::update_gui() {
    using namespace ImGui;

    //////          Debug controls window           //////
    Begin("Debug control", NULL, ImGuiWindowFlags_NoTitleBar);
    Checkbox("Render player limbs", &renderer.draw_limbs);
    Checkbox("Render player body", &renderer.draw_body);
    Checkbox("Render bones", &renderer.draw_bones);
    Checkbox("Render wireframe", &renderer.draw_wireframe);
    Checkbox("Render colliders", &renderer.draw_colliders);
    Checkbox("Render leg splines", &renderer.draw_leg_splines);

    NewLine();
    Checkbox("Use constant delta time", &game_config.use_const_delta_time);
    Checkbox("Step mode", &game_config.step_mode);
    PushItemWidth(100);
    DragFloat("Game speed", &game_config.speed, 0.1f, 0.0f, 100.0f, "%.2f");
    DragFloat("gravity", &game_config.gravity);

    // Camera
    glm::vec2 camera_center = renderer.camera_center();
    DragFloat2("Camera position", (float*)&camera_center, 1.0f, 0.0f, 0.0f,
               "%.f");

    float zoom_factor = renderer.zoom_factor();
    DragFloat("Camera zoom", &zoom_factor, 0.1f, 0.001f, 100.0f, "%.2f");

    renderer.update_camera(camera_center, zoom_factor);

    NewLine();
    Text("Mode");
    RadioButton("Play", (int*)&game_mode, GameMode::PLAY);
    RadioButton("Spline editor", (int*)&game_mode, GameMode::SPLINE_EDITOR);
    RadioButton("Level editor", (int*)&game_mode, GameMode::LEVEL_EDITOR);

    NewLine();
    Text("Player statics");
    DragFloat("ground_hover_distance", &Player::GROUND_HOVER_DISTANCE);
    DragFloat("jump_force", &Player::JUMP_FORCE);
    DragFloat("max_walk_speed", &Player::MAX_WALK_SPEED);
    DragFloat("hit_speed_multiplier", &Player::HIT_SPEED_MULTIPLIER);
    DragFloat("hitstun_duration_multiplier",
              &Player::HITSTUN_DURATION_MULTIPLIER);

    {
        Text("Display player windows");
        static bool player_window_open[2] = {true, true};
        for (size_t i = 0; i < NUM_PLAYERS; ++i) {
            char label[10];
            sprintf_s(label, "Player %zd", i);
            Checkbox(label, &player_window_open[i]);
            if (player_window_open[i]) {
                player_window_open[i] = players[i].display_debug_ui_window(i);
            }
        }
    }

    NewLine();
    Text("Animation controls");
    DragFloat("Step distance multiplier",
              &players[0].animator.step_distance_multiplier, 1.0f, 0.0f, 0.0f,
              "%.1f");
    DragFloat2("Interpolation speed min/max",
               &players[0].animator.interpolation_speed_multiplier.min, 0.01f);
    PopItemWidth();

    End();
}