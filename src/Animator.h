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

struct Animation {
    static const size_t MAX_NAME_LENGTH = 32;
    static const size_t NUM_SPLINES = 8;

    char name[MAX_NAME_LENGTH];
    Spline splines[NUM_SPLINES];

    enum SplineIndex {
        LEFT_ARM_FORWARD = 0,
        LEFT_ARM_BACKWARD = 1,
        RIGHT_ARM_FORWARD = 2,
        RIGHT_ARM_BACKWARD = 3,
        LEFT_LEG_FORWARD = 4,
        LEFT_LEG_BACKWARD = 5,
        RIGHT_LEG_FORWARD = 6,
        RIGHT_LEG_BACKWARD = 7
    };
};

enum AnimationIndex { IDLE = 0, WALK = 1, RUN = 2 };

struct LimbAnimator {
    Bone* bones[2];
    BoneRestrictions bone_restrictions[2];

    // 3 different animations, consisting of 2 splines each (forward/backward)
    Spline* splines[3];
    float spline_interpolation_factor = 0.0f;
    float lerp_interpolation_factor = 1.0f;

    static const float LERP_SPEED_MULTIPLIER;
    // @CLEANUP: Rename this?
    static const struct WalkingSpeedMultiplier {
        float MIN, MAX;
    } WALKING_SPEED_MULTIPLIER;

    static const float IDLE_LERP_SPEED_MULTIPLIER;

    glm::vec2 target_pos, tip_pos;
    glm::vec2 last_tip_movement;

    float target_rotations[2];

    bool is_walking = false;
    enum AnimationState {
        MOVE_FORWARD = 0,
        MOVE_BACKWARD = 1
    } animation_state = MOVE_FORWARD;

    VertexArray<DebugShader::Vertex> target_point_vao;

    LimbAnimator() {}
    LimbAnimator(Bone* b1, Bone* b2, Spline* splines_[3],
                 BoneRestrictions restrictions[2] = nullptr);

    void update(float delta_time, float movement_speed);
};

class WalkAnimator {
    Animation animations[3];
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