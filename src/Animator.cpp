#pragma once
#include "pch.h"
#include "Animator.h"
#include "Util.h"
#include "Mesh.h"

const float LimbAnimator::animation_speed = 0.1f;

LimbAnimator::LimbAnimator(Bone* b1, Bone* b2) {
    bones[0] = b1;
    bones[1] = b2;
    target_pos = glm::vec4{0.0f, 0.0f, 0.0f, 0.0f};

    // Init render data for rendering target_pos_model_space as a point
    GLuint index = 0;

    vao.init(&index, 1, NULL, 1);
}

void LimbAnimator::update() {
    // Return if no target position was set
    if (glm::length(target_pos) == 0.0f) {
        return;
    }

    // NOTE: Assumes that bone[0] has no rotating parents
    target_pos_bone_sapce[0] =
        bones[0]->inverse_bind_pose_transform * target_pos;

    target_pos_bone_sapce[1] = bones[1]->inverse_bind_pose_transform *
                               glm::inverse(bones[1]->parent->get_transform()) *
                               target_pos;

    float target_distance = glm::length(target_pos_bone_sapce[0]);
    float target_rotation[2];

    if (target_distance > bones[0]->length + bones[1]->length) {
        // Target out of reach
        // Get angle between local up (y-axis) and target position
        target_rotation[0] =
            atan2f(target_pos_bone_sapce[0].y, target_pos_bone_sapce[0].x) -
            degToRad(90.0f);
        target_rotation[1] =
            atan2f(target_pos_bone_sapce[1].y, target_pos_bone_sapce[1].x) -
            degToRad(90.0f);
    } else {
        float target_distance2 = target_distance * target_distance;
        float length2[2] = {bones[0]->length * bones[0]->length,
                            bones[1]->length * bones[1]->length};

        float cosAngle0 = (target_distance2 + length2[0] + length2[1]) /
                          (2 * target_distance2 * length2[0]);
        target_rotation[0] =
            acosf(cosAngle0) -
            atan2f(target_pos_bone_sapce[0].x, target_pos_bone_sapce[0].y);

        float cosAngle1 = (length2[1] + length2[0] - target_distance2) /
                          (2.0f * length2[1] * length2[0]);
        target_rotation[1] = 180.0f - acosf(cosAngle1);
    }

    bones[0]->rotation =
        lerp(bones[0]->rotation, target_rotation[0], animation_speed);

    bones[1]->rotation =
        lerp(bones[1]->rotation, target_rotation[1], animation_speed);
}