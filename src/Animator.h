#pragma once
#include "pch.h"
#include "VertexArray.h"
#include "Spline.h"

struct Bone;
struct RiggedMesh;

enum AnimState { STANDING, WALKING };

struct BoneRestrictions {
    float min_rotation, max_rotation;
};

struct LimbAnimator {
    Bone* bones[2];
    BoneRestrictions bone_restrictions[2];

    // Points to array of two splines, one for moving the limb forward and one
    // for moving it back
    Spline* splines;
    float spline_interpolation_factor = 0.0f;
    float lerp_interpolation_factor = 1.0f;

    static const float lerp_speed_multiplier;
    // @CLEANUP: Rename this?
    static const struct WalkSpeedMultiplier {
        float min, max;
    } walking_speed_multiplier;

    glm::vec2 target_pos, tip_pos;
    glm::vec2 last_tip_movement;

    float target_rotations[2];

    VertexArray<DebugShader::Vertex> target_point_vao;

    LimbAnimator() {}
    LimbAnimator(Bone* b1, Bone* b2, Spline* s,
                 BoneRestrictions restrictions[2] = nullptr);

    void update(float delta_time, float movement_speed);

    enum AnimationState {
        MOVE_FORWARD = 0,
        MOVE_BACKWARD = 1,
        NEUTRAL = 2
    } animation_state = NEUTRAL;
};

class WalkAnimator {
    Spline splines[16];
    Bone* spine;

    static const float max_spine_rotation;

  public:
    SplineEditor spline_editor;
    LimbAnimator limb_animators[4];
    bool arm_follows_mouse = false;

    enum Limb { LEFT_ARM = 0, RIGHT_ARM = 1, LEFT_LEG = 2, RIGHT_LEG = 3 };
    Limb grounded_leg_index = LEFT_LEG;

  private:
    enum {
        LEG_FORWARD = 0,
        LEG_BACKWARD = 1,
        ARM_FORWARD = 2,
        ARM_BACKWARD = 3
    };

    enum { NEUTRAL, LEFT_LEG_UP, RIGHT_LEG_UP } leg_state;

  public:
    void init(const Entity* parent, RiggedMesh& mesh);
    void update(float delta_time, float walking_speed, AnimState state,
                bool& new_grounded_state, glm::vec2 mouse_pos = {0.0f, 0.0f},
                bool mouse_down = false);
    void render(const Renderer& renderer);
};