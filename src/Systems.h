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
                          const MouseKeyboardInput& mkb,
                          const RenderData& render_data) {
    if (mkb.mouse_button(1)) {
        player.rigged_mesh.arm_animators[1].target_pos =
            inverse(player.model) *
            glm::vec4(
                static_cast<float>(mkb.mouse_pos.x),
                static_cast<float>(render_data.window_size.y - mkb.mouse_pos.y),
                0.0f, 1.0f);
    }
    for (auto& anim : player.rigged_mesh.arm_animators) {
        anim.update(delta_time);
    }

    if (!player.grounded) {
        player.pos.y -= player.gravity;
    }

    // Find closest point on gorund
    // Since ground is flat,

    // Walking animation
    if (mkb.key[SDL_SCANCODE_LEFT]) {
        // Left leg
        auto& anim0 = player.rigged_mesh.leg_animators[0];
        anim0.second_phase = false;
        anim0.target_pos =
            anim0.bones[1]->bind_pose_transform * anim0.bones[1]->tail;

        // TODO: Calculate the point better, on a circle around the bone
        anim0.target_pos.x += anim0.step_length / 2.0f;
        anim0.target_pos.y += 0.07f;

        // Right leg
        auto& anim1 = player.rigged_mesh.leg_animators[1];
        anim1.second_phase = true;
        anim1.target_pos =
            anim1.bones[1]->bind_pose_transform * anim1.bones[1]->tail;

        // TODO: Calculate the point better, on a circle around the bone
        anim1.target_pos.x += anim1.step_length / 2.0f;
        anim1.target_pos.y += 0.07f;
    }
    for (auto& anim : player.rigged_mesh.leg_animators) {
        anim.update(delta_time);
    }
}

inline void update_gui(SDL_Window* window, RenderData& render_data,
                       GameConfig& game_config, const Player& player) {
    using namespace ImGui;
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame(window);
    NewFrame();

    // Debug controls window
    Begin("Debug control", NULL, ImGuiWindowFlags_NoTitleBar);
    Checkbox("Render player.model", &render_data.draw_models);
    Checkbox("Render wireframes", &render_data.draw_wireframes);
    Checkbox("Render bones", &render_data.draw_bones);

    NewLine();
    SetNextItemWidth(100);
    DragFloat("Game speed", &game_config.speed, 0.1f, 0.0f, 100.0f, "%.2f");
    Checkbox("Step mode", &game_config.step_mode);

    End();

    // Limb data display window
    Begin("Limb data", NULL, ImGuiWindowFlags_NoTitleBar);
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

    char label[64];
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
                1.0f); // orange-ish
    ground.vao.draw(GL_TRIANGLES);

    // Translate first to multiply translation from the left to the scaled
    // player.model
    // This resolves to model * translation * scale
    player.model = translate(mat4(1.0f), vec3(player.pos, 0.0f));
    player.model = scale(player.model, vec3(100.0f, 100.0f, 1.0f));

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
