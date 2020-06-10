#pragma once
#include "pch.h"
#include "Player.h"
#include "Game.h"

void Player::init(glm::vec3 position, glm::vec3 scale_,
                  const char* texture_path, const char* mesh_path,
                  Gamepad* gamepad) {
    Entity::init(position, scale_);
    tex = Texture::load_from_file(texture_path);
    rigged_mesh.load_from_file(mesh_path);
    animator.init(this, rigged_mesh);
    gamepad_input = gamepad;

    anim_state = AnimState::STANDING;

    grounded = false;
}

void Player::update(float delta_time, const BoxCollider& ground,
                    const MouseKeyboardInput& input) {
    if (spline_edit_mode) {
        animator.spline_editor.update(input);
        animator.spline_editor
            .update_gui(); // @BUG: If this function is refactored into
                           // SplineEditor::update(), the GUI does not work
                           // correctly anymore.
        return;
    }

    if (!grounded) {
        pos.y -= gravity * delta_time;
    }

    //////          Arm animation           //////
    if (input.mouse_button(1)) {
        animator.arm_animators[1].target_pos = glm::vec4(
            world_to_local_space(glm::vec3(input.mouse_world_pos(), 0.0f)),
            1.0f);
        // inverse(model) * glm::vec4(input.mouse_world_pos(), 0.0f, 1.0f);
    }

    //////          Walking animation           //////
    if (input.key(SDL_SCANCODE_LEFT) || input.key(SDL_SCANCODE_RIGHT)) {
        anim_state = WALKING;
        if ((input.key(SDL_SCANCODE_LEFT) && facing_right) ||
            (input.key(SDL_SCANCODE_RIGHT) && !facing_right)) {
            facing_right = !facing_right;
            scale.x *= -1.0f;
        }
    } else {
        anim_state = STANDING;
    }

    animator.update(delta_time, walking_speed, anim_state);

    //////          Collision Detection         //////
    if (!grounded) {
        // Find closest point on gorund
        auto& leg_anims = animator.leg_animators;

        glm::vec4 foot_pos_world[2];
        float distance_to_ground[2];

        for (size_t i = 0; i < 2; ++i) {
            foot_pos_world[i] = local_to_world_space(leg_anims[i].foot_pos);
            distance_to_ground[i] =
                foot_pos_world[i].y - ground.pos.y - ground.half_ext.y;
        }

        size_t closer_to_ground =
            distance_to_ground[0] < distance_to_ground[1] ? 0 : 1;

        // Set grounded status
        if (distance_to_ground[closer_to_ground] <= 0.0f) {
            pos.y -= distance_to_ground[closer_to_ground];
            grounded = true;
        } else {
            grounded = false;
        }
    }

    if (grounded) {
        // @CLEANUP: This is so ugly with all the casting
        glm::vec3 move =
            scale * static_cast<glm::vec3>(
                        animator.leg_animators[animator.grounded_leg_index]
                            .last_foot_movement);
        pos -= move;
    }

    update_model_matrix();
}

void Player::render(const RenderData& render_data) {

    RiggedMesh& rm = rigged_mesh;

    // Calculate bone transforms from their rotations
    size_t bone_count = rm.bones.size();
    glm::mat4* bone_transforms = new glm::mat4[bone_count];
    for (size_t i = 0; i < bone_count; ++i) {
        auto& b = rm.bones[i];
        glm::mat4 rotation_matrix =
            rotate(glm::mat4(1.0f), b.rotation, glm::vec3(0.0f, 0.0f, 1.0f));

        bone_transforms[i] = b.get_transform();
    }

    // Calculate vertex posistions for rendering
    for (size_t i = 0; i < rm.vertices.size(); ++i) {
        Vertex vert = rm.vertices[i];
        rm.shader_vertices[i].uv_coord = vert.uv_coord;

        glm::mat4 bone =
            bone_transforms[vert.bone_index[0]] * vert.bone_weight[0] +
            bone_transforms[vert.bone_index[1]] * vert.bone_weight[1];

        rm.shader_vertices[i].pos = bone * glm::vec4(vert.position, 1.0f);
    }

    rm.vao.update_vertex_data(rm.shader_vertices);

    glUseProgram(render_data.rigged_shader.id);
    glUniformMatrix4fv(render_data.rigged_shader.model_loc, 1, GL_FALSE,
                       value_ptr(model));

    // Render player model
    if (render_data.draw_models) {

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, tex.id);

        rm.vao.draw(GL_TRIANGLES);
    }

    glUseProgram(render_data.debug_shader.id);
    glUniformMatrix4fv(render_data.debug_shader.model_loc, 1, GL_FALSE,
                       value_ptr(model));

    // Render wireframes
    if (render_data.draw_wireframes) {
        glUseProgram(render_data.debug_shader.id);
        glUniform4f(render_data.debug_shader.color_loc, 0.0f, 0.0f, 1.0f,
                    1.0f); // Red

        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        rm.vao.draw(GL_TRIANGLES);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    // Render bones
    if (render_data.draw_bones) {
        for (size_t i = 0; i < rm.bones.size(); ++i) {
            rm.bones_shader_vertices[i * 2].pos =
                bone_transforms[i] *
                rm.bones[i]
                    .bind_pose_transform[3]; // Renders (0.0f, 0.0f, 0.0f)
                                             // in the bones local space
            rm.bones_shader_vertices[i * 2 + 1].pos =
                bone_transforms[i] * rm.bones[i].bind_pose_transform *
                rm.bones[i].tail;
        }

        rm.bones_vao.update_vertex_data(rm.bones_shader_vertices);

        glUseProgram(render_data.debug_shader.id);
        glUniform4f(render_data.debug_shader.color_loc, 1.0f, 0.0f, 0.0f,
                    1.0f); // Blue

        glLineWidth(2.0f);
        rm.bones_vao.draw(GL_LINES);

        rm.bones_vao.draw(GL_POINTS);
    }

    // Render Splines
    if (render_data.draw_splines || spline_edit_mode)
        animator.spline_editor.render(render_data);

    // Render animator target positions
    animator.render(render_data);
}