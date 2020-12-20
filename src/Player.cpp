#pragma once
#include "pch.h"
#include "Player.h"
#include "Game.h"
#include "level/Collider.h"

void Player::init(glm::vec3 position, glm::vec3 scale_,
                  const char* texture_path, const char* model_path,
                  const Gamepad* pad, const std::list<BoxCollider>& colliders) {
    Entity::init(position, scale_);
    texture.load_from_file(texture_path);
    load_character_model_from_file(model_path, body_mesh, rigged_mesh);
    animator.init(this, rigged_mesh, colliders);
    SDL_assert(pad);
    gamepad = pad;
}

void Player::update(float delta_time, const std::list<BoxCollider>& colliders,
                    const MouseKeyboardInput& input) {
    auto right_stick_input = gamepad->stick(StickID::LEFT);

    if (input.key(SDL_SCANCODE_LEFT) || input.key(SDL_SCANCODE_RIGHT)) {
        if ((input.key(SDL_SCANCODE_LEFT) && facing_right) ||
            (input.key(SDL_SCANCODE_RIGHT) && !facing_right)) {
            walk_speed = 1.0f;
            facing_right = !facing_right;
            scale.x *= -1.0f;
        }
    } else if (right_stick_input.x != 0.0f) {
        walk_speed = std::abs(right_stick_input.x);
        if ((right_stick_input.x < 0.0f && facing_right) ||
            (right_stick_input.x > 0.0f && !facing_right)) {
            facing_right = !facing_right;
            scale.x *= -1.0f;
            update_model_matrix();
        }
    } else {
        walk_speed = 0.0f;
    }

    animator.update(delta_time, walk_speed, gamepad->stick(StickID::RIGHT),
                    colliders);

    update_model_matrix();

    // NEXT: player speed needs to decrease somehow
    if (std::abs(velocity.x) < max_walk_speed) {
        if (facing_right) {
            velocity.x =
                std::min(velocity.x + walk_speed * max_walk_acceleration,
                         max_walk_speed);
        } else {
            velocity.x =
                std::max(velocity.x - walk_speed * max_walk_acceleration,
                         -max_walk_speed);
        }
    }

    if (!grounded) {
        velocity.y -= gravity;
    }
}

void Player::render(const Renderer& renderer) {
    // Calculate bone transforms from their rotations
    glm::mat3 bone_transforms[RiggedShader::NUMBER_OF_BONES];
    SDL_assert(rigged_mesh.bones.size() <= RiggedShader::NUMBER_OF_BONES);
    for (size_t i = 0; i < rigged_mesh.bones.size(); ++i) {
        bone_transforms[i] = rigged_mesh.bones[i].transform();
    }

    if (renderer.draw_limbs) {
        renderer.rigged_shader.use();
        renderer.rigged_shader.set_model(&model);
        renderer.rigged_shader.set_bone_transforms(bone_transforms);

        renderer.rigged_shader.set_texture(texture);

        rigged_mesh.vao.draw(GL_TRIANGLES);
    }

    if (renderer.draw_body) {
        renderer.textured_shader.use();
        renderer.textured_shader.set_model(&model);

        renderer.textured_shader.set_texture(texture);

        body_mesh.vao.draw(GL_TRIANGLES);
    }

    if (renderer.draw_wireframe) {
        renderer.debug_shader.use();
        renderer.debug_shader.set_model(&model);
        rigged_mesh.vao.draw(GL_LINE_LOOP);
    }

    // Render bones
    if (renderer.draw_bones) {
        renderer.bone_shader.set_model(&model);
        renderer.bone_shader.set_color(&Color::RED);
        renderer.bone_shader.set_bone_transforms(bone_transforms);

        glLineWidth(2.0f);
        rigged_mesh.bones_vao.draw(GL_LINES);
        rigged_mesh.bones_vao.draw(GL_POINTS);
    }

    // Render animator target positions
    animator.render(renderer);
}

bool Player::is_facing_right() const noexcept { return facing_right; }