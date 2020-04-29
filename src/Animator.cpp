#pragma once
#include "pch.h"
#include "Animator.h"
#include "Util.h"
#include "Mesh.h"

static void resolve_ik(Bone* const bones[2], glm::vec4 target_pos,
                       float* target_rotations) {
    // NOTE: Assumes that bone[0] has no rotating parents
    glm::vec4 target_pos_bone_space[2];
    target_pos_bone_space[0] =
        bones[0]->inverse_bind_pose_transform * target_pos;

    target_pos_bone_space[1] = bones[1]->inverse_bind_pose_transform *
                               glm::inverse(bones[1]->parent->get_transform()) *
                               target_pos;

    float target_distance = glm::length(
        static_cast<glm::vec3>(target_pos - bones[0]->bind_pose_transform[3]));

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
        target_rotations[0] =
            atan2f(target_pos_bone_space[0].y, target_pos_bone_space[0].x) -
            acosf(cosAngle0) - degToRad(90.0f);

        float cosAngle1 = (length2[0] + length2[1] - target_distance2) /
                          (2.0f * bones[0]->length * bones[1]->length);
        target_rotations[1] = degToRad(180.0f) - acosf(cosAngle1);
    }

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
                              animation_speed * delta_time);

    bones[1]->rotation = lerp(bones[1]->rotation, target_rotations[1],
                              animation_speed * delta_time);
}

LegAnimator::LegAnimator(Bone* b1, Bone* b2) {
    bones[0] = b1;
    bones[1] = b2;
    target_pos = glm::vec4{0.0f, 0.0f, 0.0f, 0.0f};

    // Init render data for rendering target_pos_model_space as a point
    GLuint index = 0;

    vao.init(&index, 1, NULL, 1);

    // // Init render data for rendering the spline that the foot follows
    // GLuint indices[debug_render_steps * 2];
    // for (GLuint i = 0; i < debug_render_steps; ++i) {
    //     indices[i * 2] = i;
    //     indices[i * 2 + 1] = i + 1;
    // }

    // vao.init(indices, debug_render_steps * 2, NULL, 20);
}

void LegAnimator::update(float delta_time) {
    // Return if no target position was set
    if (glm::length(target_pos) == 0.0f) {
        return;
    }

    time_since_start += delta_time;

    if (time_since_start > step_duration) {
        time_since_start -= step_duration;
        second_phase = !second_phase;
    }

    float point_in_animation = time_since_start / step_duration;
    if (!second_phase) {
        float sine = sinf(point_in_animation * PI);
        foot_pos = target_pos - glm::vec4(point_in_animation * step_length,
                                          -sine * step_height, 0.0f, 0.0f);
    } else {
        foot_pos =
            target_pos - glm::vec4(1.0f - point_in_animation * step_length,
                                   0.0f, 0.0f, 0.0f);
    }

    float target_rotations[2];
    resolve_ik(bones, foot_pos, target_rotations);

    bones[0]->rotation = target_rotations[0];
    bones[1]->rotation = target_rotations[1];
}