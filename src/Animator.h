#pragma once
#include <list>
#include "rendering/VertexArray.h"
#include "Spline.h"
#include "Collider.h"

class Bone;
struct RiggedMesh;
class Game;
class Player;

// NOTE: Not all of these splines are actually relevant for the players
// animations. For example, the legs just stick to one point on the ground in
// their idle animation. This is in part due to changes that the animation
// system went through, but I also don't see a good way to restructure it so it
// only contains the relevant splines.
struct SplineSet {
    Spline walk[2];
    Spline run[2];
    Spline idle[2];
};

struct Limb {
    Spline spline; // In local space

    Bone* bones[2];

    glm::vec2 origin() const;
    float length() const;
};

class Animator {
  public:
    // Indices of the splines for the animations in SplineSet
    enum SplineIndex { LEG_FORWARD = 0, LEG_BACKWARD = 1 };

    // Indices of the limbs for the limbs member variable.
    enum LegIndex { LEFT_LEG = 0, RIGHT_LEG = 1 };

    void init(const Player* parent_, RiggedMesh& mesh,
              const std::list<AABB>& colliders);
    void update(float delta_time, float walking_speed,
                glm::vec2 right_stick_input, const std::list<AABB>& colliders);

    glm::vec2 tip_pos(LegIndex limb_index) const;
    const Bone* weapon() const noexcept;

  private:
    const Player* parent;
    SplineEditor* spline_editor;
    Bone* weapon_;
    float max_weapon_length = 3.0f;

    glm::vec2 right_arm_target_position;

    // The splines are all in the player's local space.
    SplineSet spline_prototypes;
    float step_distance_world;
    float spine_rotation_target;

    // float pelvis_height;

    Limb limbs[2];

    float interpolation_factor_between_splines;
    float interpolation_factor_on_spline;

    enum LegState {
        NEUTRAL,
        LEFT_LEG_UP,
        RIGHT_LEG_UP
    } leg_state,
        last_leg_state;

    float step_distance_multiplier = 100.0f;

    struct InterpolationSpeedMultiplier {
        float min = 0.02f, max = 0.08f;
    } interpolation_speed_multiplier;

    void set_new_splines(float walking_speed, const std::list<AABB>& colliders);

    void interpolate_splines(glm::vec2 dst[Spline::NUM_POINTS],
                             SplineIndex spline_index) const;

    // The debug UI needs to access the private members of this class. Letting
    // Game access them this way seems cleaner to me then writing a bunch of
    // getters/setters that are only used in one place.
    friend Game;
    friend Player;
};