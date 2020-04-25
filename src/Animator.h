#pragma once
#include "pch.h"
#include "VertexArrayData.h"

struct Bone;

struct LimbAnimator {
    Bone* bones[2];

    glm::vec4 target_pos;

    VertexArrayData<DebugShaderVertex> vao;

    glm::vec4 target_pos_bone_sapce[2];

    const static float animation_speed;

    LimbAnimator(Bone* b1, Bone* b2);

    void update();
};