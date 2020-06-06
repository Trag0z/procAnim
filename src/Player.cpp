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

void Player::update(float delta_time, const BoxCollider& ground,
                    const MouseKeyboardInput& input) {
    //////          Arm animation           //////
    if (input.mouse_button(1)) {
        rigged_mesh.arm_animators[1].target_pos =
            inverse(model) * glm::vec4(input.mouse_world_pos(), 0.0f, 1.0f);
    }
    for (auto& arm_anim : rigged_mesh.arm_animators) {
        arm_anim.update(delta_time);
    }

    if (!grounded) {
        pos.y -= gravity * delta_time;
    }

    //////          Collision Detection         //////
    // Find closest point on gorund
    auto& leg_anims = rigged_mesh.leg_animators;

    glm::vec4 foot_pos_world[2];
    float distance_to_ground[2];

    for (size_t i = 0; i < 2; ++i) {
        foot_pos_world[i] = model * leg_anims[i].foot_pos;
        distance_to_ground[i] =
            foot_pos_world[i].y - ground.pos.y - ground.half_ext.y;
    }

    size_t closer_to_ground =
        distance_to_ground[0] < distance_to_ground[1] ? 0 : 1;

    // Set grounded status
    if (distance_to_ground[closer_to_ground] <= 0.0f) {
        pos.y -= distance_to_ground[closer_to_ground];

        grounded = true;
        leg_anims[closer_to_ground].grounded = true;
        leg_anims[closer_to_ground ^ 1].grounded = false;
    } else {
        grounded = false;
        leg_anims[0].grounded = false;
        leg_anims[1].grounded = false;
    }

    //////          Walking animation           //////
    int move_direction = 0;
    if (input.key(SDL_SCANCODE_LEFT)) {
        move_direction -= 1;
    }
    if (input.key(SDL_SCANCODE_RIGHT)) {
        move_direction += 1;
    }

    if (move_direction != 0) {
        if ((facing_right && move_direction == -1) ||
            (!facing_right && move_direction == 1)) {
            facing_right = !facing_right;
            scale.x *= -1.0f;
        }

        switch (anim_state) {
        case Player::STANDING:
            // Start walking
            leg_anims[0].set_target_foot_pos(LegAnimator::RAISED);
            leg_anims[1].set_target_foot_pos(LegAnimator::NEUTRAL);

            anim_state = Player::WALKING;
            walk_state = Player::LEFT_UP;
            break;
        case Player::WALKING:
            if (leg_anims[0].has_reached_target_rotation() &&
                leg_anims[1].has_reached_target_rotation()) {
                switch (walk_state) {
                case Player::LEFT_UP:
                    leg_anims[0].set_target_foot_pos(LegAnimator::FRONT);
                    leg_anims[1].set_target_foot_pos(LegAnimator::BACK);

                    walk_state = Player::LEFT_DOWN;
                    break;
                case Player::LEFT_DOWN:
                    leg_anims[0].set_target_foot_pos(LegAnimator::NEUTRAL);
                    leg_anims[1].set_target_foot_pos(LegAnimator::RAISED);

                    walk_state = Player::RIGHT_UP;
                    break;
                case Player::RIGHT_UP:
                    leg_anims[0].set_target_foot_pos(LegAnimator::BACK);
                    leg_anims[1].set_target_foot_pos(LegAnimator::FRONT);

                    walk_state = Player::RIGHT_DOWN;
                    break;
                case Player::RIGHT_DOWN:
                    leg_anims[0].set_target_foot_pos(LegAnimator::RAISED);
                    leg_anims[1].set_target_foot_pos(LegAnimator::NEUTRAL);

                    walk_state = Player::LEFT_UP;
                    break;
                }
            }
            break;
        default:
            SDL_TriggerBreakpoint();
            break;
        }
    } else if (anim_state == Player::WALKING) {
        // Stop walking, reset to bind pose positions
        leg_anims[0].target_pos = leg_anims[0].bones[1]->bind_pose_transform *
                                  leg_anims[0].bones[1]->tail;

        leg_anims[1].target_pos = leg_anims[1].bones[1]->bind_pose_transform *
                                  leg_anims[1].bones[1]->tail;
        anim_state = Player::STANDING;
    }

    for (auto& anim : leg_anims) {
        anim.update(delta_time, walking_speed);
    }

    if (grounded) {
        // @CLEANUP: This is so ugly with all the casting
        glm::vec2 move = static_cast<glm::vec2>(
            scale * static_cast<glm::vec3>(
                        leg_anims[closer_to_ground].last_foot_movement));
        pos -= move;
    }
}