#pragma once
#include "pch.h"
#include "VertexArrayData.h"

struct Bone;

struct ArmAnimator {
    Bone* bones[2];

    glm::vec4 target_pos;

    VertexArrayData<DebugShaderVertex> vao;

    const static float animation_speed;

    ArmAnimator(Bone* b1, Bone* b2);

    void update(float delta_time);
};

struct LegAnimator {
    Bone* bones[2];

    glm::vec4 target_pos, foot_pos;
    float step_length = 1.0f, step_height = 0.5f;
    float step_duration = 120.0f;
    float time_since_start = 0.0f;

    VertexArrayData<DebugShaderVertex> vao;

    const static GLuint debug_render_steps = 20;

    LegAnimator(Bone* b1, Bone* b2);

    void update(float delta_time);
};