#pragma once
#include "pch.h"
#include "VertexArrayData.h"

struct Bone;

struct ArmAnimator {
    Bone* bones[2];

    glm::vec4 target_pos;

    VertexArrayData<DebugShaderVertex> vao;

    const static float animation_speed;

    ArmAnimator() {}
    ArmAnimator(Bone* b1, Bone* b2);

    void update(float delta_time);
};

struct LegAnimator {
    Bone* bones[2];

    glm::vec4 target_pos, foot_pos;
    glm::vec4 last_foot_movement;
    static float step_length, step_height;
    bool grounded;

    float target_rotation[2];

    VertexArrayData<DebugShaderVertex> vao;

    LegAnimator() {}
    LegAnimator(Bone* b1, Bone* b2);

    void update(float delta_time, float walking_speed);
    bool has_reached_target_rotation() const;

    enum TargetFootPosition { NEUTRAL, RAISED, FRONT, BACK };
    void set_target_foot_pos(TargetFootPosition pos);
};