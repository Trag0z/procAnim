#pragma once
#include "pch.h"
#include "Player.h"
#include "Game.h"
#include "level/Collider.h"

void Player::init(glm::vec3 position, glm::vec3 scale_,
                  const char* texture_path, const char* mesh_path,
                  Gamepad* pad) {
    Entity::init(position, scale_);
    tex.load_from_file(texture_path);
    rigged_mesh.load_from_file(mesh_path);
    animator.init(this, rigged_mesh);
    SDL_assert(pad);
    gamepad = pad;

    anim_state = AnimState::STANDING;

    grounded = false;
}

void Player::update(float delta_time, const std::list<BoxCollider>& colliders,
                    const MouseKeyboardInput& input) {
    if (!grounded) {
        velocity.y -= gravity * delta_time;
        position_ += velocity;
        update_model_matrix();
    }

    //////          Walking animation           //////
    auto stick = gamepad->stick(StickID::LEFT);
    if (input.key(SDL_SCANCODE_LEFT) || input.key(SDL_SCANCODE_RIGHT)) {
        anim_state = WALKING;
        if ((input.key(SDL_SCANCODE_LEFT) && facing_right) ||
            (input.key(SDL_SCANCODE_RIGHT) && !facing_right)) {
            facing_right = !facing_right;
            scale.x *= -1.0f;
        }
    } else if (stick.x != 0.0f) {
        anim_state = WALKING;
        walking_speed = std::abs(stick.x);
        if ((stick.x < 0.0f && facing_right) ||
            (stick.x > 0.0f && !facing_right)) {
            facing_right = !facing_right;
            scale.x *= -1.0f;
        }
    } else {
        anim_state = STANDING;
    }

    bool not_grounded_anymore = false;
    glm::vec2 local_mouse_pos = world_to_local_space(input.mouse_pos_world());
    animator.update(delta_time, walking_speed, anim_state, not_grounded_anymore,
                    local_mouse_pos, input.mouse_button(1));

    //////          Collision Detection         //////
    if (not_grounded_anymore || !grounded) {
        // Find closest point on gorund
        glm::vec2 foot_pos_world[2];

        foot_pos_world[0] = local_to_world_space(
            animator.limb_animators[WalkAnimator::LEFT_LEG].tip_pos);
        foot_pos_world[1] = local_to_world_space(
            animator.limb_animators[WalkAnimator::RIGHT_LEG].tip_pos);

        float distance_to_ground = 0.0f;
        for (auto& coll : colliders) {
            for (size_t i = 0; i < 2; ++i) {
                if (coll.is_inside_rect(foot_pos_world[i])) {
                    auto coll_top = (coll.position.y + coll.half_ext.y);
                    distance_to_ground = std::min(
                        distance_to_ground, foot_pos_world[i].y - coll_top);
                }
            }
        }

        // Set grounded status
        if (distance_to_ground < 0.0f) {
            position_.y -= distance_to_ground;
            grounded = true;
        }
    }

    if (grounded) {
        glm::vec2 move =
            scale * animator.limb_animators[animator.grounded_leg_index]
                        .last_tip_movement;
        if (not_grounded_anymore) {
            grounded = false;
            velocity = move;
        } else {
            velocity = glm::vec2(0.0f);
            position_ -= move;
        }
    }

    update_model_matrix();
}

void Player::render(const Renderer& renderer) {
    RiggedMesh& rm = rigged_mesh;

    // Calculate bone transforms from their rotations
    size_t bone_count = rm.bones.size();
    glm::mat3* bone_transforms = new glm::mat3[bone_count];
    for (size_t i = 0; i < bone_count; ++i) {
        bone_transforms[i] = rm.bones[i].get_transform();
    }

    // Calculate vertex posistions for rendering
    for (size_t i = 0; i < rm.vertices.size(); ++i) {
        RiggedVertex vert = rm.vertices[i];
        rm.shader_vertices[i].uv_coord = vert.uv_coord;

        glm::mat3 bone =
            bone_transforms[vert.bone_index[0]] * vert.bone_weight[0] +
            bone_transforms[vert.bone_index[1]] * vert.bone_weight[1];

        rm.shader_vertices[i].pos = bone * glm::vec3(vert.position, 1.0f);
    }

    rm.vao.update_vertex_data(rm.shader_vertices);

    renderer.rigged_shader.use();
    renderer.rigged_shader.set_model(&model);

    // Render player model
    if (renderer.draw_models) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, tex.id);

        rm.vao.draw(GL_TRIANGLES);
    }

    renderer.debug_shader.use();
    renderer.debug_shader.set_model(&model);

    // Render wireframes
    if (renderer.draw_wireframes) {
        renderer.debug_shader.set_color(&Colors::RED);

        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        rm.vao.draw(GL_TRIANGLES);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    // Render bones
    if (renderer.draw_bones) {
        for (size_t i = 0; i < rm.bones.size(); ++i) {
            rm.bones_shader_vertices[i * 2].pos =
                bone_transforms[i] *
                rm.bones[i]
                    .bind_pose_transform[2]; // Renders (0.0f, 0.0f, 0.0f)
                                             // in the bones local space
            rm.bones_shader_vertices[i * 2 + 1].pos =
                bone_transforms[i] * rm.bones[i].bind_pose_transform *
                glm::vec3(rm.bones[i].tail, 1.0f);
        }

        rm.bones_vao.update_vertex_data(rm.bones_shader_vertices);

        renderer.debug_shader.set_color(&Colors::BLUE);

        glLineWidth(2.0f);
        rm.bones_vao.draw(GL_LINES);

        rm.bones_vao.draw(GL_POINTS);
    }

    delete[] bone_transforms;

    // Render animator target positions
    animator.render(renderer);
}