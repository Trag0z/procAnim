#pragma once
#include "pch.h"
#include "Player.h"
#include "Game.h"
#include "level/Collider.h"

void Player::init(glm::vec3 position, glm::vec3 scale_,
                  const char* texture_path, const char* mesh_path,
                  const Gamepad* pad, const std::list<BoxCollider>& colliders) {
    Entity::init(position, scale_);
    tex.load_from_file(texture_path);
    mesh.load_from_file(mesh_path);
    animator.init(this, mesh, colliders);
    SDL_assert(pad);
    gamepad = pad;
}

void Player::update(float delta_time, const std::list<BoxCollider>& colliders,
                    const MouseKeyboardInput& input) {

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

    animator.update(delta_time, walking_speed, colliders);

    position_ = animator.pelvis_pos();

    update_model_matrix();
}

void Player::render(const Renderer& renderer) {
    // Calculate bone transforms from their rotations
    glm::mat3 bone_transforms[RiggedShader::NUMBER_OF_BONES];
    SDL_assert(mesh.bones.size() < RiggedShader::NUMBER_OF_BONES);
    for (size_t i = 0; i < mesh.bones.size(); ++i) {
        bone_transforms[i] = mesh.bones[i].transform();
    }

    renderer.rigged_shader.use();
    renderer.rigged_shader.set_model(&model);
    renderer.rigged_shader.set_bone_transforms(bone_transforms);

    // Render player model
    if (renderer.draw_models) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, tex.id);

        mesh.vao.draw(GL_TRIANGLES);
    }

    renderer.debug_shader.use();
    renderer.debug_shader.set_model(&model);

    // Render wireframes
    // TODO: This doesn't adjust to bone movements
    if (renderer.draw_wireframes) {
        renderer.debug_shader.set_color(&Color::BLUE);

        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        mesh.vao.draw(GL_LINES);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    // Render bones
    if (renderer.draw_bones) {
        renderer.rigged_debug_shader.set_model(&model);
        renderer.rigged_debug_shader.set_color(&Color::RED);
        renderer.rigged_debug_shader.set_bone_transforms(bone_transforms);

        glLineWidth(2.0f);
        mesh.bones_vao.draw(GL_LINES);
        mesh.bones_vao.draw(GL_POINTS);
    }

    // Render animator target positions
    animator.render(renderer);
}

bool Player::is_facing_right() const noexcept { return facing_right; }