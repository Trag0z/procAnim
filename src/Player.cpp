#pragma once
#include "pch.h"
#include "Player.h"
#include "Game.h"
#include "level/Collider.h"

void Player::init(glm::vec3 position_, glm::vec3 scale_,
                  const char* texture_path, const char* mesh_path,
                  const Gamepad* pad) {
    Entity::init(position_, scale_);
    tex.load_from_file(texture_path);
    rigged_mesh.load_from_file(mesh_path);
    animator.init(this, rigged_mesh);
    SDL_assert(pad);
    gamepad = pad;

    grounded = false;
}

void Player::update(float delta_time, const std::list<BoxCollider>& colliders,
                    const MouseKeyboardInput& input) {
    if (!grounded) {
        velocity.y -= gravity * delta_time;
        position += velocity;
        update_model_matrix();
    }

    //////          Walking animation           //////
    auto stick = gamepad->stick(StickID::LEFT);

    if (input.key(SDL_SCANCODE_LEFT) || input.key(SDL_SCANCODE_RIGHT)) {
        if ((input.key(SDL_SCANCODE_LEFT) && facing_right) ||
            (input.key(SDL_SCANCODE_RIGHT) && !facing_right)) {
            walking_speed = 1.0f;
            facing_right = !facing_right;
            scale.x *= -1.0f;
        }
    } else if (stick.x != 0.0f) {
        walking_speed = std::abs(stick.x);
        if ((stick.x < 0.0f && facing_right) ||
            (stick.x > 0.0f && !facing_right)) {
            facing_right = !facing_right;
            scale.x *= -1.0f;
            update_model_matrix();
        }
    } else {
        walking_speed = 0.0f;
    }

    glm::vec2 local_mouse_pos = world_to_local_space(input.mouse_pos_world());
    animator.update(delta_time, walking_speed, input, colliders);

    //////          Collision Detection         //////
    if (!grounded) { // NOTE: grounded never gets set to false once it's true
        // Find closest point on gorund
        glm::vec2 foot_pos_world[2];

        foot_pos_world[0] =
            local_to_world_space(animator.get_tip_pos(Animator::LEFT_LEG));
        foot_pos_world[1] =
            local_to_world_space(animator.get_tip_pos(Animator::RIGHT_LEG));

        float distance_to_ground = 0.0f;
        for (auto& coll : colliders) {
            for (size_t i = 0; i < 2; ++i) {
                if (coll.encloses_point(foot_pos_world[i])) {
                    distance_to_ground =
                        std::min(distance_to_ground,
                                 foot_pos_world[i].y - coll.top_edge());
                }
            }
        }

        // Set grounded status
        if (distance_to_ground < 0.0f) {
            position.y -= distance_to_ground;
            grounded = true;
        }
    }

    if (grounded) {
        velocity = glm::vec2(0.0f);
        position -= scale * animator.get_last_ground_movement();
    }

    update_model_matrix();
}

void Player::render(const Renderer& renderer) {
    RiggedMesh& rm = rigged_mesh;

    // Calculate bone transforms from their rotations
    glm::mat3 bone_transforms[RiggedShader::NUMBER_OF_BONES];
    SDL_assert(rigged_mesh.bones.size() < RiggedShader::NUMBER_OF_BONES);
    for (size_t i = 0; i < rigged_mesh.bones.size(); ++i) {
        bone_transforms[i] = rigged_mesh.bones[i].get_transform();
    }

    renderer.rigged_shader.use();
    renderer.rigged_shader.set_model(&model);
    renderer.rigged_shader.set_bone_transforms(bone_transforms);

    // Render player model
    if (renderer.draw_models) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, tex.id);

        rm.vao.draw(GL_TRIANGLES);
    }

    renderer.debug_shader.use();
    renderer.debug_shader.set_model(&model);

    // Render wireframes
    // TODO: This doesn't adjust to bone movements
    if (renderer.draw_wireframes) {
        renderer.debug_shader.set_color(&Color::RED);

        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        rm.vao.draw(GL_TRIANGLES);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    // Render bones
    if (renderer.draw_bones) {
        for (size_t i = 0; i < rm.bones.size(); ++i) {
            rm.bones_shader_vertices[i * 2].pos =
                bone_transforms[i] *
                rm.bones[i].bind_pose_transform[2]; // Renders (0.0f, 0.0f,
                                                    // 0.0f) in the bones local
                                                    // space
            rm.bones_shader_vertices[i * 2 + 1].pos =
                bone_transforms[i] * rm.bones[i].bind_pose_transform *
                glm::vec3(rm.bones[i].tail, 1.0f);
        }

        rm.bones_vao.update_vertex_data(rm.bones_shader_vertices);

        renderer.debug_shader.set_color(&Color::BLUE);

        glLineWidth(2.0f);
        rm.bones_vao.draw(GL_LINES);

        rm.bones_vao.draw(GL_POINTS);
    }

    // Render animator target positions
    animator.render(renderer);
}

bool Player::is_facing_right() const noexcept { return facing_right; }