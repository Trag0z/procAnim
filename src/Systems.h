#pragma once
#include "pch.h"
#include "Mesh.h"
#include <imgui/imgui.h>
#include <imgui/imgui_impl_sdl.h>
#include <imgui/imgui_impl_opengl3.h>

inline void poll_inputs(MouseKeyboardInput& mkb,
                        std::array<GamepadInput, 4>& pads) {
    SDL_PumpEvents();

    // Update mouse
    Uint32 newMouseButton =
        SDL_GetMouseState(&mkb.mouse_pos.x, &mkb.mouse_pos.y);
    for (size_t i = 0; i < mkb.num_mouse_buttons; ++i) {
        if (newMouseButton & SDL_BUTTON(i)) {
            if (!(mkb.mouse_button_map & SDL_BUTTON(i))) {
                mkb.mouse_button_down_map |= SDL_BUTTON(i);
            } else {
                mkb.mouse_button_down_map &= ~SDL_BUTTON(i);
            }
            mkb.mouse_button_map |= SDL_BUTTON(i);
            mkb.mouse_button_up_map &= ~SDL_BUTTON(i);
        } else if (mkb.mouse_button_map & SDL_BUTTON(i)) {
            mkb.mouse_button_map &= ~SDL_BUTTON(i);
            mkb.mouse_button_down_map &= ~SDL_BUTTON(i);
            mkb.mouse_button_up_map |= SDL_BUTTON(i);
        } else {
            mkb.mouse_button_up_map &= ~SDL_BUTTON(i);
        }
    }

    // Update keyboard keys
    for (int i = 0; i < mkb.num_keys; ++i) {
        if (mkb.sdl_keyboard[i]) {
            if (!mkb.key[i]) {
                mkb.key_down[i] = true;
            } else {
                mkb.key_down[i] = false;
            }
            mkb.key[i] = true;
        } else if (mkb.key[i]) {
            mkb.key[i] = false;
            mkb.key_down[i] = false;
            mkb.key_up[i] = true;
        } else {
            mkb.key_up[i] = false;
        }
    }

    // Update gamepads
    for (size_t padIndex = 0;
         padIndex < static_cast<size_t>(GamepadInput::num_gamepads);
         ++padIndex) {
        GamepadInput& pad = pads[padIndex];

        // Check if gamepad is still valid
        SDL_assert(pad.sdl_ptr);

        // Poll all the axes on this pad
        Sint16 tempAxis;
        for (size_t i = 0; i < GamepadInput::num_axes; ++i) {

            // If it's a trigger, normalize and set the value
            if (i == SDL_CONTROLLER_AXIS_TRIGGERLEFT ||
                i == SDL_CONTROLLER_AXIS_TRIGGERRIGHT) {
                pad.axis[i] =
                    static_cast<float>(SDL_GameControllerGetAxis(
                        pad.sdl_ptr, static_cast<SDL_GameControllerAxis>(i))) /
                    32767.0F;
                continue;
            }

            // If it's a stick, calculate and set it's value
            tempAxis = SDL_GameControllerGetAxis(
                pad.sdl_ptr, static_cast<SDL_GameControllerAxis>(i));
            if (tempAxis > -GamepadInput::stick_deadzone_in &&
                tempAxis < GamepadInput::stick_deadzone_in) {
                pad.axis[i] = 0.0F;
            } else if (tempAxis > GamepadInput::stick_deadzone_out) {
                pad.axis[i] = 1.0F;
            } else if (tempAxis < -GamepadInput::stick_deadzone_out) {
                pad.axis[i] = -1.0F;
            } else {
                if (tempAxis > 0) {
                    pad.axis[i] =
                        static_cast<float>(tempAxis -
                                           GamepadInput::stick_deadzone_in) /
                        static_cast<float>(GamepadInput::stick_deadzone_out -
                                           GamepadInput::stick_deadzone_in);
                } else {
                    pad.axis[i] =
                        static_cast<float>(tempAxis +
                                           GamepadInput::stick_deadzone_in) /
                        static_cast<float>(GamepadInput::stick_deadzone_out -
                                           GamepadInput::stick_deadzone_in);
                }
            }
            // Invert Y axis cause it's the wrong way by default
            if (i == SDL_CONTROLLER_AXIS_LEFTY ||
                i == SDL_CONTROLLER_AXIS_RIGHTY)
                pad.axis[i] *= -1.0f;
        }

        // Poll all the buttons on this pad
        for (U32 i = 0; i < GamepadInput::num_buttons; ++i) {
            if (SDL_GameControllerGetButton(
                    pad.sdl_ptr, static_cast<SDL_GameControllerButton>(i))) {
                if (!pad.button(i)) {
                    setNthBitTo(pad.button_down_map, i, 1);
                } else {
                    setNthBitTo(pad.button_down_map, i, 0);
                }
                setNthBitTo(pad.button_map, i, 1);
            } else if (pad.button(i)) {
                setNthBitTo(pad.button_down_map, i, 0);
                setNthBitTo(pad.button_up_map, i, 1);
                setNthBitTo(pad.button_map, i, 0);
            } else {
                setNthBitTo(pad.button_up_map, i, 0);
            }
        }
    }
}

inline void update_player(float delta_time, Player& player,
                          const BoxCollider& ground,
                          const MouseKeyboardInput& mkb,
                          const RenderData& render_data) {
    //////          Arm animation           //////
    if (mkb.mouse_button(1)) {
        player.rigged_mesh.arm_animators[1].target_pos =
            inverse(player.model) *
            glm::vec4(
                static_cast<float>(mkb.mouse_pos.x),
                static_cast<float>(render_data.window_size.y - mkb.mouse_pos.y),
                0.0f, 1.0f);
    }
    for (auto& arm_anim : player.rigged_mesh.arm_animators) {
        arm_anim.update(delta_time);
    }

    if (!player.grounded) {
        player.pos.y -= player.gravity;
    }

    //////          Collision Detection         //////
    // Find closest point on gorund
    auto& leg_anims = player.rigged_mesh.leg_animators;

    glm::vec4 foot_pos_world[2];
    float distance_to_ground[2];

    for (size_t i = 0; i < 2; ++i) {
        foot_pos_world[i] = player.model * leg_anims[i].foot_pos;
        distance_to_ground[i] =
            foot_pos_world[i].y - ground.pos.y - ground.half_ext.y;
    }

    size_t closer_to_ground =
        distance_to_ground[0] < distance_to_ground[1] ? 0 : 1;
    // Should be the same as:
    // if (distance_to_ground[0] < distance_to_ground[1]) {
    //     closer_to_ground = 0;
    // } else {
    //     closer_to_ground = 1;
    // }

    // Set grounded status
    if (distance_to_ground[closer_to_ground] <= 0.0f) {
        player.pos.y -= distance_to_ground[closer_to_ground];

        player.grounded = true;
        leg_anims[closer_to_ground].grounded = true;
        leg_anims[closer_to_ground ^ 1].grounded = false;
    } else {
        player.grounded = false;
        leg_anims[0].grounded = false;
        leg_anims[1].grounded = false;
    }

    //////          Walking animation           //////
    int move_direction = 0;
    if (mkb.key[SDL_SCANCODE_LEFT]) {
        move_direction -= 1;
    }
    if (mkb.key[SDL_SCANCODE_RIGHT]) {
        move_direction += 1;
    }

    if (move_direction != 0) {
        if ((player.facing_left && move_direction == 1) ||
            (!player.facing_left && move_direction == -1)) {
            player.facing_left = !player.facing_left;
            player.scale.x *= -1.0f;
        }

        switch (player.anim_state) {
        case Player::STANDING:
            // Start walking
            leg_anims[0].set_target_foot_pos(LegAnimator::RAISED);
            leg_anims[1].set_target_foot_pos(LegAnimator::NEUTRAL);

            player.anim_state = Player::WALKING;
            player.walk_state = Player::LEFT_UP;
            break;
        case Player::WALKING:
            if (leg_anims[0].has_reached_target_rotation() &&
                leg_anims[1].has_reached_target_rotation()) {
                switch (player.walk_state) {
                case Player::LEFT_UP:
                    leg_anims[0].set_target_foot_pos(LegAnimator::LEFT);
                    leg_anims[1].set_target_foot_pos(LegAnimator::RIGHT);

                    player.walk_state = Player::LEFT_DOWN;
                    break;
                case Player::LEFT_DOWN:
                    leg_anims[0].set_target_foot_pos(LegAnimator::NEUTRAL);
                    leg_anims[1].set_target_foot_pos(LegAnimator::RAISED);

                    player.walk_state = Player::RIGHT_UP;
                    break;
                case Player::RIGHT_UP:
                    leg_anims[0].set_target_foot_pos(LegAnimator::RIGHT);
                    leg_anims[1].set_target_foot_pos(LegAnimator::LEFT);

                    player.walk_state = Player::RIGHT_DOWN;
                    break;
                case Player::RIGHT_DOWN:
                    leg_anims[0].set_target_foot_pos(LegAnimator::RAISED);
                    leg_anims[1].set_target_foot_pos(LegAnimator::NEUTRAL);

                    player.walk_state = Player::LEFT_UP;
                    break;
                }
            }
            break;
        default:
            SDL_TriggerBreakpoint();
            break;
        }
    } else if (player.anim_state == Player::WALKING) {
        // Stop walking, reset to bind pose positions
        leg_anims[0].target_pos = leg_anims[0].bones[1]->bind_pose_transform *
                                  leg_anims[0].bones[1]->tail;

        leg_anims[1].target_pos = leg_anims[1].bones[1]->bind_pose_transform *
                                  leg_anims[1].bones[1]->tail;
        player.anim_state = Player::STANDING;
    }

    for (auto& anim : leg_anims) {
        anim.update(delta_time, player.walking_speed);
    }

    if (player.grounded) {
        // So ugly with all the casting
        glm::vec2 move = static_cast<glm::vec2>(
            player.scale * static_cast<glm::vec3>(
                               leg_anims[closer_to_ground].last_foot_movement));
        player.pos -= move;
    }
}

inline void update_gui(SDL_Window* window, RenderData& render_data,
                       GameConfig& game_config, const Player& player) {
    using namespace ImGui;
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame(window);
    NewFrame();

    //////          Debug controls window           //////
    Begin("Debug control", NULL, ImGuiWindowFlags_NoTitleBar);
    Checkbox("Render player.model", &render_data.draw_models);
    Checkbox("Render wireframes", &render_data.draw_wireframes);
    Checkbox("Render bones", &render_data.draw_bones);

    NewLine();
    SetNextItemWidth(100);
    DragFloat("Game speed", &game_config.speed, 0.1f, 0.0f, 100.0f, "%.2f");
    Checkbox("Step mode", &game_config.step_mode);

    End();

    //////          Limb data display window            //////
    Begin("Limb data", NULL, ImGuiWindowFlags_NoTitleBar);

    char label[64];

    sprintf_s(label, "% 6.1f, % 6.1f", player.pos.x, player.pos.y);
    Text("Player position: ");
    SameLine();
    DragFloat2("Player position", (float*)&player.pos, 1.0f, 0.0f, 0.0f,
               "% .2f");

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

    for (const auto& anim : player.rigged_mesh.arm_animators) {
        glm::vec4 target_world_pos = player.model * anim.target_pos;
        sprintf_s(label, "%6.1f, %6.1f", target_world_pos.x,
                  target_world_pos.y);
        Text(label);
        NextColumn();
    }
    for (const auto& anim : player.rigged_mesh.leg_animators) {
        glm::vec4 target_world_pos = player.model * anim.target_pos;
        sprintf_s(label, "%6.1f, %6.1f", target_world_pos.x,
                  target_world_pos.y);
        Text(label);
        NextColumn();
    }
    Columns(1);
    Separator();

    NewLine();
    Text("Limb data");
    Columns(3);
    Separator();
    Text("Name");
    NextColumn();
    Text("Rotation deg/rad");
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

        glm::vec4 tail_world_pos = player.model * bone.get_transform() *
                                   bone.bind_pose_transform * bone.tail;
        sprintf_s(label, "% 7.1f, % 7.1f", tail_world_pos.x, tail_world_pos.y);
        Text(label);
        NextColumn();
    }
    Columns(1);
    Separator();

    End();
}

inline void render(SDL_Window* window, const RenderData& render_data,
                   Player& player, BoxCollider& ground) {
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    using namespace glm;

    // Render ground
    ground.model = translate(mat4(1.0f), vec3(ground.pos, 0.0f));

    for (size_t i = 0; i < ground.vertices.size(); ++i) {
        ground.shader_vertices[i].pos =
            render_data.projection * ground.model * ground.vertices[i];
    }
    ground.vao.update_vertex_data(ground.shader_vertices);

    glUseProgram(render_data.debug_shader.id);
    glUniform4f(render_data.debug_shader.color_loc, 1.0f, 0.5f, 0.2f,
                1.0f); // ugly orange-ish
    ground.vao.draw(GL_TRIANGLES);

    // Translate first to multiply translation from the left to the scaled
    // player.model
    // This resolves to model * translation * scale
    player.model = player.model = translate(mat4(1.0f), vec3(player.pos, 0.0f));
    player.model = scale(player.model, player.scale);

    RiggedMesh& rm = player.rigged_mesh;

    // Calculate bone transforms from their rotations
    size_t bone_count = rm.bones.size();
    mat4* bone_transforms = new mat4[bone_count];
    for (size_t i = 0; i < bone_count; ++i) {
        auto& b = rm.bones[i];
        mat4 rotation_matrix =
            rotate(mat4(1.0f), b.rotation, vec3(0.0f, 0.0f, 1.0f));

        bone_transforms[i] = b.get_transform();
    }

    // Calculate vertex posistions for rendering
    for (size_t i = 0; i < rm.vertices.size(); ++i) {
        Vertex vert = rm.vertices[i];
        rm.shader_vertices[i].uv_coord = vert.uv_coord;

        mat4 bone = bone_transforms[vert.bone_index[0]] * vert.bone_weight[0] +
                    bone_transforms[vert.bone_index[1]] * vert.bone_weight[1];

        rm.shader_vertices[i].pos = render_data.projection * player.model *
                                    bone * vec4(vert.position, 1.0f);
    }

    rm.vao.update_vertex_data(rm.shader_vertices);

    // Render player model
    if (render_data.draw_models) {
        glUseProgram(render_data.rigged_shader.id);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, player.tex.id);

        rm.vao.draw(GL_TRIANGLES);
    }

    // Render wireframes
    if (render_data.draw_wireframes) {
        glUseProgram(render_data.debug_shader.id);
        glUniform4f(render_data.debug_shader.color_loc, 0.0f, 0.0f, 1.0f,
                    1.0f); // red

        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        rm.vao.draw(GL_TRIANGLES);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    // Render bones
    if (render_data.draw_bones) {
        for (size_t i = 0; i < rm.bones.size(); ++i) {
            rm.bones_shader_vertices[i * 2].pos =
                render_data.projection * player.model * bone_transforms[i] *
                rm.bones[i]
                    .bind_pose_transform[3]; // Renders (0.0f, 0.0f, 0.0f) in
                                             // the bones local space
            rm.bones_shader_vertices[i * 2 + 1].pos =
                render_data.projection * player.model * bone_transforms[i] *
                rm.bones[i].bind_pose_transform * rm.bones[i].tail;
        }

        rm.bones_vao.update_vertex_data(rm.bones_shader_vertices);

        glUseProgram(render_data.debug_shader.id);
        glUniform4f(render_data.debug_shader.color_loc, 1.0f, 0.0f, 0.0f,
                    1.0f); // blue

        glLineWidth(2.0f);
        rm.bones_vao.draw(GL_LINES);
        glLineWidth(1.0f);

        rm.bones_vao.draw(GL_POINTS);
    }

    // Render animator target positions
    for (auto& anim : rm.arm_animators) {
        if (anim.target_pos.w == 0.0f)
            continue;

        vec4 render_pos =
            render_data.projection * player.model * anim.target_pos;

        anim.vao.update_vertex_data(1, reinterpret_cast<DebugShaderVertex*>(
                                           &render_pos)); // ugly, but it works

        glUseProgram(render_data.debug_shader.id);
        glUniform4f(render_data.debug_shader.color_loc, 0.0f, 1.0f, 0.0f, 1.0f);

        anim.vao.draw(GL_POINTS);
    }

    for (auto& anim : rm.leg_animators) {
        if (anim.target_pos.w == 0.0f)
            continue;

        vec4 render_pos = render_data.projection * player.model * anim.foot_pos;

        anim.vao.update_vertex_data(1, reinterpret_cast<DebugShaderVertex*>(
                                           &render_pos)); // ugly, but it works

        glUseProgram(render_data.debug_shader.id);
        glUniform4f(render_data.debug_shader.color_loc, 0.0f, 1.0f, 0.0f, 1.0f);

        anim.vao.draw(GL_POINTS);
    }

    // Unbind vao for error safety
    glBindVertexArray(0);

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    SDL_GL_SwapWindow(window);
};
