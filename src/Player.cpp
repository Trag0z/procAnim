#pragma once
#include "pch.h"
#include "Player.h"
#include "Game.h"
#include "Collider.h"

float Player::GROUND_HOVER_DISTANCE = 160.0f;
float Player::JUMP_FORCE = 30.0f;
float Player::MAX_WALK_SPEED = 10.0f;
float Player::MAX_AIR_ACCELERATION = 0.5f;
float Player::MAX_AIR_SPEED = 10.0f;
float Player::HIT_SPEED_MULTIPLIER = 0.2f;
float Player::HIT_COOLDOWN = 30.0f;

static const struct {
    u32 jump = SDL_CONTROLLER_BUTTON_RIGHTSHOULDER;
    u32 jump_alt = SDL_CONTROLLER_BUTTON_A;
} button_map;

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
    if (time_since_last_hit < HIT_COOLDOWN) {
        time_since_last_hit += delta_time;
    }

    if (state == HITSTUN) {
        update_model_matrix();
        return;
    }

    auto left_stick_input = gamepad->stick(StickID::LEFT);

    if (input.key(SDL_SCANCODE_LEFT) || input.key(SDL_SCANCODE_RIGHT)) {
        if ((input.key(SDL_SCANCODE_LEFT) && facing_right) ||
            (input.key(SDL_SCANCODE_RIGHT) && !facing_right)) {
            walk_speed = 1.0f;
            facing_right = !facing_right;
            scale.x *= -1.0f;
        }
    } else if (left_stick_input.x != 0.0f) {
        walk_speed = std::abs(left_stick_input.x);
        if ((left_stick_input.x < 0.0f && facing_right) ||
            (left_stick_input.x > 0.0f && !facing_right)) {
            facing_right = !facing_right;
            scale.x *= -1.0f;
            update_model_matrix();
        }
    } else {
        walk_speed = 0.0f;
    }

    last_weapon_collider = weapon_collider;
    animator.update(delta_time, walk_speed, gamepad->stick(StickID::RIGHT),
                    colliders);
    auto weapon = animator.weapon();
    weapon_collider = LineCollider(local_to_world_space(weapon->head()),
                                   local_to_world_space(weapon->tail()));

    update_model_matrix();

    // Movement
    if (state == STANDING || state == WALKING) {
        SDL_assert(grounded);
        velocity.x = left_stick_input.x * MAX_WALK_SPEED;

        if (gamepad->button_down(button_map.jump) ||
            gamepad->button_down(button_map.jump_alt)) {
            velocity.y += JUMP_FORCE;
            state = FALLING;
            grounded = false;
        }
    } else if (state == FALLING) {
        SDL_assert(!grounded);

        velocity.x =
            clamp(velocity.x + left_stick_input.x * MAX_AIR_ACCELERATION,
                  -MAX_AIR_SPEED, MAX_AIR_SPEED);
    }
}

bool Player::is_facing_right() const noexcept { return facing_right; }

CircleCollider Player::body_collider() const noexcept {
    return {local_to_world_space(body_collider_.position),
            std::abs(local_to_world_scale(body_collider_.radius))};
    // Using std::abs() here is a kinda hacky way to get
    // around the fact that when the player is facing
    // left, it's scale is negative on the x-axis
}