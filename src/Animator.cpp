#pragma once
#include "pch.h"
#include "Animator.h"
#include "Util.h"
#include "Mesh.h"

// Takes a target_pos in model space and finds two target_rotations for the
// bones, so that the tail of bone[1] is at (or at the closest possible point
// to) target_pos
static void resolve_ik(Bone* const bones[2], glm::vec4 target_pos,
                       float* target_rotations) {
    // We need the target positions before the bone's rotation is applied to
    // find the new, absolute rotation. Therefore, we use the
    // inverse_bind_pose_transform
    glm::vec4 target_pos_bone_space[2];
    target_pos_bone_space[0] = bones[0]->inverse_bind_pose_transform *
                               glm::inverse(bones[0]->parent->get_transform()) *
                               target_pos;

    target_pos_bone_space[1] = bones[1]->inverse_bind_pose_transform *
                               glm::inverse(bones[0]->get_transform()) *
                               target_pos;

    float target_distance = glm::length(
        static_cast<glm::vec3>(target_pos - bones[0]->bind_pose_transform[3]));

    // THe resulting rotations here are applied counter clockwise, but why?
    if (target_distance > bones[0]->length + bones[1]->length) {
        // Target out of reach
        // Get angle between local up (y-axis) and target position
        target_rotations[0] =
            atan2f(target_pos_bone_space[0].y, target_pos_bone_space[0].x) -
            degToRad(90.0f);
        target_rotations[1] =
            atan2f(target_pos_bone_space[1].y, target_pos_bone_space[1].x) -
            degToRad(90.0f);
    } else {
        float target_distance2 = target_distance * target_distance;
        float length2[2] = {bones[0]->length * bones[0]->length,
                            bones[1]->length * bones[1]->length};

        float cosAngle0 = (target_distance2 + length2[0] - length2[1]) /
                          (2 * target_distance * bones[0]->length);

        float atan =
            atan2f(target_pos_bone_space[0].y, target_pos_bone_space[0].x);
        float acos = acosf(cosAngle0);
        target_rotations[0] = atan - acos - PI * 0.5f;

        float cosAngle1 = (length2[0] + length2[1] - target_distance2) /
                          (2.0f * bones[0]->length * bones[1]->length);
        float acos1 = acosf(cosAngle1);
        target_rotations[1] =
            PI - acos1; // TODO: The argument for degToRad() should be
                        // 180, but then the arm overshoots the position
                        // by a bit. Find out why that is! 175 gives
                        // almost pixel perfect results for arms.

        /*if (target_rotations[1] > 0.0f) {
            target_rotations[0] = -atan2f(target_pos_bone_space[0].y,
                                          target_pos_bone_space[0].x) +
                                  acosf(cosAngle0) + degToRad(180.0f);
            target_rotations[1] = degToRad(180.0f) + acosf(cosAngle1);
        }*/
    }

    // If the target rotation is more than 180 degrees away from the current
    // rotation, choose the shorter way around the circle
    // NOTE: Is this necessary if the point is out of reach?
    if (target_rotations[0] - bones[0]->rotation > PI) {
        target_rotations[0] -= 2.0f * PI;
    } else if (target_rotations[0] - bones[0]->rotation < -PI) {
        target_rotations[0] += 2.0f * PI;
    }

    if (target_rotations[1] - bones[1]->rotation > PI) {
        target_rotations[1] -= 2.0f * PI;
    } else if (target_rotations[1] - bones[1]->rotation < -PI) {
        target_rotations[1] += 2.0f * PI;
    }
}

const float ArmAnimator::animation_speed = 0.1f;

ArmAnimator::ArmAnimator(Bone* b1, Bone* b2) {
    bones[0] = b1;
    bones[1] = b2;
    target_pos = glm::vec4{0.0f, 0.0f, 0.0f, 0.0f};

    // Init render data for rendering target_pos_model_space as a point
    GLuint index = 0;

    vao.init(&index, 1, NULL, 1);
}

void ArmAnimator::update(float delta_time) {
    // Return if no target position was set
    if (glm::length(target_pos) == 0.0f) {
        return;
    }

    float target_rotations[2];
    resolve_ik(bones, target_pos, target_rotations);

    bones[0]->rotation = lerp(bones[0]->rotation, target_rotations[0],
                              std::min(1.0f, animation_speed * delta_time));

    bones[1]->rotation = lerp(bones[1]->rotation, target_rotations[1],
                              std::min(1.0f, animation_speed * delta_time));
}

float LegAnimator::step_length = 1.5f;
float LegAnimator::step_height = 0.7f;

LegAnimator::LegAnimator(Bone* b1, Bone* b2) {
    bones[0] = b1;
    bones[1] = b2;
    target_pos = glm::vec4{0.0f, 0.0f, 0.0f, 0.0f};
    foot_pos = bones[1]->get_transform() * bones[1]->bind_pose_transform *
               bones[1]->tail;
    last_foot_movement = glm::vec4(0.0f);
    grounded = false;

    // Init render data for rendering target_pos as a point
    GLuint index = 0;

    vao.init(&index, 1, NULL, 1);
}

void LegAnimator::update(float delta_time, float walking_speed) {
    // If no target position was set, update foot_pos and return
    if (glm::length(target_pos) == 0.0f) {
        foot_pos = bones[1]->get_transform() * bones[1]->bind_pose_transform *
                   bones[1]->tail;
        return;
    }

    resolve_ik(bones, target_pos, target_rotation);

    // NOTE: delta_time is always 1.0f if we hit the target framerate and the
    // game is running at normal speed multiplier
    // NOTE: These lerps never reach 1.0f because walking_speed * delta_time is
    // always about 0.2
    bones[0]->rotation = lerp(bones[0]->rotation, target_rotation[0],
                              std::min(1.0f, walking_speed * delta_time));

    bones[1]->rotation = lerp(bones[1]->rotation, target_rotation[1],
                              std::min(1.0f, walking_speed * delta_time));

    // Update foot_pos
    glm::vec4 new_foot_pos = bones[1]->get_transform() *
                             bones[1]->bind_pose_transform * bones[1]->tail;
    last_foot_movement = new_foot_pos - foot_pos;
    foot_pos = new_foot_pos;
}

bool LegAnimator::has_reached_target_rotation() const {
    return std::abs(bones[0]->rotation - target_rotation[0]) < 0.1f &&
           std::abs(bones[1]->rotation - target_rotation[1]) < 0.1f;
}

void LegAnimator::set_target_foot_pos(TargetFootPosition pos) {
    target_pos = bones[1]->bind_pose_transform * bones[1]->tail;
    if (pos == RAISED) {
        target_pos.x += step_length / 4.0f;
        target_pos.y += step_height;
    } else if (pos == FRONT) {
        target_pos.x += step_length / 2.0f;
        target_pos.y += 0.24f;
    } else if (pos == BACK) {
        target_pos.x -= step_length / 2.0f;
    }
}