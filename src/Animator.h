#pragma once
#include "pch.h"
#include "VertexArray.h"
#include "Spline.h"
#include "level/Collider.h"

struct Bone;
struct RiggedMesh;
class Game;
class Player;

// @TODO: Rename to AnimationState // @CLEANUP: What?
struct BoneRestrictions {
    float min_rotation, max_rotation;
};

struct SplineSet {
    Spline walk[4];
    Spline run[4];
    Spline idle[4];
};

struct Limb {
    float target_rotations[2];
    Spline spline; // In parent's local space

    Bone* bones[2];
    BoneRestrictions bone_restrictions[2];

    glm::vec2 origin() const;
    float length() const;
};

class Animator {
  public:
    enum SplineIndex {
        LEG_FORWARD = 0,
        LEG_BACKWARD = 1,
        ARM_FORWARD = 2,
        ARM_BACKWARD = 3
    };
    enum LimbIndex { LEFT_ARM = 0, RIGHT_ARM = 1, LEFT_LEG = 2, RIGHT_LEG = 3 };

    void init(const Player* parent_, RiggedMesh& mesh);
    void update(float delta_time, float walking_speed,
                const MouseKeyboardInput& input,
                const std::list<BoxCollider>& colliders);
    void render(const Renderer& renderer);

    glm::vec2 get_tip_pos(LimbIndex limb_index) const;

    glm::vec2 get_last_ground_movement() const;

  private:
    const Player* parent;
    SplineEditor* spline_editor;
    Bone* spine;

    // The splines' coordinate systems originate at the head of the first bone
    // they control (i.e. the shoulder or the hip)
    SplineSet spline_prototypes;

    Limb limbs[4];

    VertexArray<DebugShader::Vertex> target_points_vao;

    float interpolation_factor_between_splines;
    float interpolation_factor_on_spline, last_interpolation_factor_on_spline;

    glm::vec2 last_ground_movement;

    bool arm_follows_mouse = false;

    static const float MAX_SPINE_ROTATION;

    // @CLEANUP: Rename this?
    static const struct WalkingSpeedMultiplier {
        float MIN, MAX;
    } WALKING_SPEED_MULTIPLIER;

    enum { NEUTRAL, LEFT_LEG_UP, RIGHT_LEG_UP } leg_state, last_leg_state;

    glm::vec2
    find_interpolated_target_point(SplineIndex spline_index) const noexcept;

    // Scans the colliders for the highest top edge that's lower than the leg's
    // hip and less far away than the current spline's target position. CAUTION:
    // The limbs' splines should be set before calling this method!
    void find_point_to_step_on(const std::list<BoxCollider>& colliders);

    // Creates a new spline for the limb using an interpolation of walk and run
    // splines with the target position as an end point. If leg_state is
    // neutral, it just uses the idle spline.
    void set_limb_spline(
        LimbIndex limb, SplineIndex spline,
        glm::vec2 target_pos_world_space = glm::vec2(0.0f)) noexcept;

    void set_new_limb_splines(const std::list<BoxCollider>& colliders);

    void interpolate_splines(glm::vec2 out[Spline::NUM_POINTS],
                             SplineIndex spline_index) const;

    friend Game;
};