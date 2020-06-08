#pragma once
#include "pch.h"
#include "VertexArrayData.h"
#include "Spline.h"

struct Bone;
struct RiggedMesh;

enum AnimState { STANDING, WALKING };

struct BoneRestrictions {
    float min_rotation, max_rotation;
};

struct ArmAnimator {
    Bone* bones[2];
    BoneRestrictions bone_restrictions[2];

    glm::vec4 target_pos;

    VertexArrayData<DebugShaderVertex> vao;

    const static float animation_speed;

    ArmAnimator() {}
    ArmAnimator(Bone* b1, Bone* b2, BoneRestrictions restrictions[2] = nullptr);

    void update(float delta_time);
};

struct LegAnimator {
    Bone* bones[2];
    BoneRestrictions bone_restrictions[2];

    const Spline* spline = nullptr;
    float current_interpolation = 0.0f;
    glm::vec4 target_pos, foot_pos;
    glm::vec4 last_foot_movement;
    bool grounded;

    float target_rotations[2];

    VertexArrayData<DebugShaderVertex> target_point_vao;
    VertexArrayData<DebugShaderVertex> circle_vao;
    static const size_t circle_segments = 30;

    LegAnimator() {}
    LegAnimator(Bone* b1, Bone* b2, BoneRestrictions restrictions[2] = nullptr);

    void update(float delta_time, float walking_speed);
};

class WalkAnimator {
  public:
    ArmAnimator arm_animators[2];
    LegAnimator leg_animators[2];
    Spline splines[4];
    SplineEditor spline_editor;

    enum {
        LEG_FORWARD = 0,
        LEG_BACKWARD = 1,
        ARM_FORWARD = 2,
        ARM_BACKWARD = 3
    };

    enum { LEFT_LEG = 0, RIGHT_LEG = 1 };

    enum { NEUTRAL, LEFT_LEG_UP, RIGHT_LEG_UP } leg_state;

    void init(const Entity* parent, RiggedMesh& mesh);
    void update(float delta_time, float walking_speed, AnimState state);
    void render(const RenderData& render_data);
};