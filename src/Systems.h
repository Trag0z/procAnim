#pragma once
#include "pch.h"
#include "Mesh.h"

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
            if (tempAxis > -GamepadInput::joy_deadzone_in &&
                tempAxis < GamepadInput::joy_deadzone_in) {
                pad.axis[i] = 0.0F;
            } else if (tempAxis > GamepadInput::joy_deadzone_out) {
                pad.axis[i] = 1.0F;
            } else if (tempAxis < -GamepadInput::joy_deadzone_out) {
                pad.axis[i] = -1.0F;
            } else {
                if (tempAxis > 0) {
                    pad.axis[i] =
                        static_cast<float>(tempAxis -
                                           GamepadInput::joy_deadzone_in) /
                        static_cast<float>(GamepadInput::joy_deadzone_out -
                                           GamepadInput::joy_deadzone_in);
                } else {
                    pad.axis[i] =
                        static_cast<float>(tempAxis +
                                           GamepadInput::joy_deadzone_in) /
                        static_cast<float>(GamepadInput::joy_deadzone_out -
                                           GamepadInput::joy_deadzone_in);
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

inline void update_player(Player& player) {
    // Arm control
    constexpr float sensitivity = 1.5f;
    auto& mesh = player.rigged_mesh;

    // Rotate upper arm
    size_t bone_index = mesh.find_bone_index("Arm_L_1");
    SDL_assert(bone_index != RiggedMesh::Bone::INDEX_NOT_FOUND);

    mesh.bones[bone_index].rotation = glm::rotate(
        mesh.bones[bone_index].rotation,
        degToRad(-sensitivity *
                 player.gamepad_input->axis[SDL_CONTROLLER_AXIS_LEFTY]),
        glm::vec3(0.0f, 0.0f, 1.0f));

    // Rotate lower arm
    bone_index = mesh.find_bone_index("Arm_L_2");
    SDL_assert(bone_index != RiggedMesh::Bone::INDEX_NOT_FOUND);

    mesh.bones[bone_index].rotation = glm::rotate(
        mesh.bones[bone_index].rotation,
        degToRad(-sensitivity *
                 player.gamepad_input->axis[SDL_CONTROLLER_AXIS_RIGHTY]),
        glm::vec3(0.0f, 0.0f, 1.0f));
}

inline void render(SDL_Window* window, RenderData render_data, Player& player) {
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    using namespace glm;

    mat4 projection = ortho(0.0f, 1920.0f, 0.0f, 1080.0f, -1.0f, 1.0f);

    // Translate first to multiply translation from the left to the scaled model
    // This resolves to model * translation * scale
    mat4 model(1.0f);
    model = translate(model, vec3(player.pos, 0.0f));
    model = scale(model, vec3(100.0f, 100.0f, 1.0f));

    RiggedMesh& rm = player.rigged_mesh;

    // Calculate bone transforms from their rotations
    size_t bone_count = rm.bones.size();
    mat4* bone_transforms = new mat4[bone_count];
    for (size_t i = 0; i < bone_count; ++i) {
        auto& b = rm.bones[i];

        if (b.parent == RiggedMesh::Bone::INDEX_NOT_FOUND) {
            bone_transforms[i] =
                inverse(b.inverse_transform) * b.rotation * b.inverse_transform;
        } else {
            // What if the parent has a parent?
            bone_transforms[i] = bone_transforms[b.parent] *
                                 inverse(b.inverse_transform) * b.rotation *
                                 b.inverse_transform;
        }
    }

    // Calculate vertex posistions for rendering
    for (size_t i = 0; i < rm.vertices.size(); ++i) {
        Vertex vert = rm.vertices[i];
        rm.shader_vertices[i].uv_coord = vert.uv_coord;

        mat4 bone = bone_transforms[vert.bone_index[0]] * vert.bone_weight[0] +
                    bone_transforms[vert.bone_index[1]] * vert.bone_weight[1];

        rm.shader_vertices[i].pos =
            projection * model * bone * vec4(vert.position, 1.0f);
    }

    glNamedBufferSubData(
        rm.vbo, 0, sizeof(RiggedMesh::ShaderVertex) * rm.shader_vertices.size(),
        rm.shader_vertices.data());
    glBindVertexArray(rm.vao);

    if (render_data.draw_wireframes) {
        glUseProgram(render_data.debug_shader.id);
        glUniform4f(render_data.debug_shader.color_loc, 0.1f, 0.1f, 1.0f, 1.0f);

        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glDrawElements(GL_TRIANGLES, rm.num_indices, GL_UNSIGNED_INT, 0);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    } else {
        glUseProgram(render_data.rigged_shader.id);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, player.tex.id);

        glDrawElements(GL_TRIANGLES, rm.num_indices, GL_UNSIGNED_INT, 0);
    }

    if (render_data.draw_bones) {
        for (size_t i = 0; i < rm.bones.size(); ++i) {
            rm.bones_shader_vertices[i * 2].pos = projection * model *
                                                  bone_transforms[i] *
                                                  vec4(rm.bones[i].head, 1.0f);
            rm.bones_shader_vertices[i * 2 + 1].pos =
                projection * model * bone_transforms[i] *
                vec4(rm.bones[i].tail, 1.0f);
        }

        glNamedBufferSubData(rm.bones_vbo, 0,
                             sizeof(RiggedMesh::DebugShaderVertex) *
                                 rm.bones_shader_vertices.size(),
                             rm.bones_shader_vertices.data());
        glBindVertexArray(rm.bones_vao);

        glUseProgram(render_data.debug_shader.id);
        glUniform4f(render_data.debug_shader.color_loc, 1.0f, 0.0f, 0.0f, 1.0f);

        glLineWidth(2.0f);
        glDrawElements(GL_LINES,
                       static_cast<GLsizei>(rm.bones_shader_vertices.size()),
                       GL_UNSIGNED_INT, 0);
        glLineWidth(1.0f);

        glDrawElements(GL_POINTS,
                       static_cast<GLsizei>(rm.bones_shader_vertices.size()),
                       GL_UNSIGNED_INT, 0);
    }

    // Unbind vao for error safety
    glBindVertexArray(0);

    SDL_GL_SwapWindow(window);
};
