#pragma once
#include "Player.h"
#include "Game.h"
#include "Collider.h"
#include <imgui/imgui.h>
#include <glm/gtc/type_ptr.hpp>

float Player::GROUND_HOVER_DISTANCE = 160.0f;
float Player::JUMP_FORCE = 30.0f;
float Player::DOUBLE_JUMP_FORCE = 15.0f;
float Player::GRAVITY = 2.0f;

float Player::WALL_JUMP_FORCE = 15.0f;
float Player::MAX_WALL_JUMP_COYOTE_TIME = 5.0f;

float Player::WALK_ACCELERATION = 1.0f;
float Player::MAX_WALK_SPEED = 10.0f;

float Player::MAX_AIR_ACCELERATION = 0.5f;
float Player::MAX_AIR_SPEED = 10.0f;

float Player::MAX_WALL_CLIMB_SPEED = 10.0f;

float Player::HIT_SPEED_MULTIPLIER = 0.2f;
float Player::MAX_HIT_COOLDOWN = 30.0f;
float Player::HITSTUN_DURATION_MULTIPLIER = 0.8f;

static const struct {
    u32 jump = SDL_CONTROLLER_BUTTON_RIGHTSHOULDER;
    u32 jump_alt = SDL_CONTROLLER_BUTTON_A;
} button_map;

void Player::init(glm::vec3 position, glm::vec3 scale_,
                  const char* texture_path, const char* model_path,
                  const Gamepad* pad, const std::list<AABB>& colliders) {
    Entity::init(position, scale_);
    texture.load_from_file(texture_path);
    load_character_model_from_file(model_path, body_mesh, rigged_mesh);
    animator.init(this, rigged_mesh, colliders);
    SDL_assert(pad);
    gamepad = pad;
}

void Player::update(float delta_time, const std::list<AABB>& colliders) {
    if (hit_cooldown > 0.0f)
        hit_cooldown -= delta_time;
    if (wall_jump_cotyote_time > 0.0f)
        wall_jump_cotyote_time -= delta_time;

    if (state == HITSTUN) {
        hitstun_duration -= delta_time;

        if (hitstun_duration > 0.0f) {
            update_model_matrix();
            return;
        } else {
            hitstun_duration = 0.0f;
            state = FALLING;
            grounded = false;
        }
    }

    const vec2 left_stick_input = gamepad->stick(StickID::LEFT);

    if (left_stick_input.x != 0.0f) {
        if ((left_stick_input.x < 0.0f && facing_right) ||
            (left_stick_input.x > 0.0f && !facing_right)) {
            facing_right = !facing_right;
            scale.x *= -1.0f;
        }
    }

    // Movement
    if (state == STANDING || state == WALKING) {
        SDL_assert(grounded);
        float target_velocity_x = left_stick_input.x * MAX_WALK_SPEED;

        if (velocity.x > target_velocity_x) {
            velocity.x =
                glm::max(target_velocity_x, velocity.x - WALK_ACCELERATION);
        } else if (velocity.x < target_velocity_x) {
            velocity.x =
                glm::min(target_velocity_x, velocity.x + WALK_ACCELERATION);
        }

        if (left_stick_input.x != 0.0f) {
            state = WALKING;
        } else {
            state = STANDING;
        }

        if (gamepad->button_down(button_map.jump) ||
            gamepad->button_down(button_map.jump_alt)) {
            velocity.x = left_stick_input.x * MAX_AIR_SPEED;
            velocity.y = JUMP_FORCE;

            state = FALLING;
            grounded = false;
        }
    } else if (state == WALL_CLING) {
        velocity.y = left_stick_input.y * MAX_WALL_CLIMB_SPEED;

        if ((wall_direction == Direction::LEFT && left_stick_input.x > 0.5f) ||
            (wall_direction == Direction::RIGHT &&
             left_stick_input.x < -0.5f)) {
            wall_jump_cotyote_time = MAX_WALL_JUMP_COYOTE_TIME;
            velocity.y = 0.0f;
            state = FALLING;
        }
    } else if (state == FALLING) {
        SDL_assert(!grounded);

        velocity.x =
            glm::clamp(velocity.x + left_stick_input.x * MAX_AIR_ACCELERATION,
                       -MAX_AIR_SPEED, MAX_AIR_SPEED);

        if (can_double_jump && (gamepad->button_down(button_map.jump) ||
                                gamepad->button_down(button_map.jump_alt))) {
            velocity.x = left_stick_input.x * MAX_AIR_SPEED;
            velocity.y = DOUBLE_JUMP_FORCE;

            can_double_jump = false;
        }
    }

    // Wall jump
    if (wall_jump_cotyote_time > 0.0f &&
        (gamepad->button_down(button_map.jump) ||
         gamepad->button_down(button_map.jump_alt))) {

        velocity = glm::normalize(left_stick_input) * WALL_JUMP_FORCE;
        state = FALLING;
    }

    // Leg and weapon animation
    last_weapon_collider = weapon_collider;
    animator.update(delta_time, velocity.x, gamepad->stick(StickID::RIGHT),
                    colliders);
    auto weapon = animator.weapon();
    {
        glm::vec2 head_world = local_to_world_space(weapon->head());
        weapon_collider =
            Segment{head_world, local_to_world_space(weapon->tail())};
    }
    update_model_matrix();
}

bool Player::is_facing_right() const noexcept { return facing_right; }

Circle Player::body_collider() const noexcept {
    return {local_to_world_space(body_collider_.center),
            std::abs(local_to_world_scale(body_collider_.radius))};
    // Using std::abs() here is a kinda hacky way to get
    // around the fact that when the player is facing
    // left, it's scale is negative on the x-axis
}

bool Player::display_debug_ui(size_t player_index) {
    using namespace ImGui;

    bool keep_open = true;
    {
        char window_name[10];
        sprintf_s(window_name, "Player %zd", player_index);
        Begin(window_name, &keep_open);
    }

    PushItemWidth(150);
    DragFloat2("position", value_ptr(position_), 1.0f, 0.0f, 0.0f, "%.2f");
    DragFloat2("velocity", value_ptr(velocity), 1.0f, 0.0f, 0.0f, "%.2f");

    Checkbox("grounded", &grounded);

    {
        const char* items[] = {"STANDING", "WALKING", "FALLING", "HITSTUN",
                               "WALL_CLING"};
        int current_state = static_cast<int>(state);
        if (Combo("state", &current_state, items, IM_ARRAYSIZE(items))) {
            state = static_cast<State>(current_state);
        }
    }

    Checkbox("facing_right", &facing_right);

    DragFloat("hit_cooldown", &hit_cooldown, 1.0f, 0.0f, 0.0f, "%.2f");
    DragFloat("hitstun_duration", &hitstun_duration, 1.0f, 0.0f, 0.0f, "%.2f");
    DragFloat("max_weapon_length", &animator.max_weapon_length);
    float current_length =
        glm::length(animator.weapon()->tail() - animator.weapon()->head());
    DragFloat("current_weapon_length", &current_length);

    PopItemWidth();
    End();

    return keep_open;
}