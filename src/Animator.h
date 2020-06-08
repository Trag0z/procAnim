#pragma once
#include "pch.h"
#include "VertexArrayData.h"
#include "Spline.h"

struct Bone;
struct RiggedMesh;

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

    glm::vec4 target_pos, foot_pos;
    glm::vec4 last_foot_movement;
    static float step_length, step_height;
    bool grounded;

    float target_rotation[2];

    VertexArrayData<DebugShaderVertex> vao;

    LegAnimator() {}
    LegAnimator(Bone* b1, Bone* b2, BoneRestrictions restrictions[2] = nullptr);

    void update(float delta_time, float walking_speed);
    bool has_reached_target_rotation() const;

    enum TargetFootPosition { NEUTRAL, RAISED, FRONT, BACK };
    void set_target_foot_pos(TargetFootPosition pos);
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

    void init(const Entity* parent, RiggedMesh& mesh);
    void update();
    void render(const RenderData& render_data, bool render_splines);
};