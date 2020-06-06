#pragma once
#include "pch.h"
#include "Player.h"

void Player::init(glm::vec2 position, glm::vec3 scale_factor,
                  const char* texture_path, const char* mesh_path,
                  Gamepad* gamepad) {
    pos = position;
    scale = scale_factor;
    tex = Texture::load_from_file(texture_path);
    rigged_mesh.load_from_file(mesh_path);
    gamepad_input = gamepad;

    anim_state = AnimState::STANDING;
    walk_state = WalkState::LEFT_UP;

    grounded = false;

    model = glm::translate(glm::mat4(1.0f), glm::vec3(pos, 0.0f));
    model = glm::scale(model, glm::vec3(100.0f, 100.0f, 1.0f));
}