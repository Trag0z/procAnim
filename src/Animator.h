#pragma once
#include "pch.h"
#include "VertexArray.h"
#include "Spline.h"
#include "level/Collider.h"

struct Bone;
struct Mesh;
class Game;
class Player;

struct SplineSet {
    Spline walk[5];
    Spline run[5];
    Spline idle[5];
};

struct Limb {
    Spline spline; // In local space

    Bone* bones[2];

    glm::vec2 origin() const;
    float length() const;
};

class Animator {
  public:
    enum SplineIndex {
        LEG_FORWARD = 0,
        LEG_BACKWARD = 1,
        ARM_FORWARD = 2,
        ARM_BACKWARD = 3,
        PELVIS = 4
    };
    enum LimbIndex { LEFT_ARM = 0, RIGHT_ARM = 1, LEFT_LEG = 2, RIGHT_LEG = 3 };

    void init(const Player* parent_, Mesh& mesh,
              const std::list<BoxCollider>& colliders);
    void update(float delta_time, float walking_speed,
                const std::list<BoxCollider>& colliders);
    void render(const Renderer& renderer);

    glm::vec2 tip_pos(LimbIndex limb_index) const;

    glm::vec2 pelvis_pos() const;

  private:
    const Player* parent;
    SplineEditor* spline_editor;
    Bone* spine;

    // The splines are all in players' local space
    SplineSet spline_prototypes;
    Spline pelvis_spline;
    float step_distance_world;
    float spine_rotation_target;

    float pelvis_height;

    Limb limbs[4];

    float interpolation_factor_between_splines;
    float interpolation_factor_on_spline;

    enum LegState {
        NEUTRAL,
        LEFT_LEG_UP,
        RIGHT_LEG_UP
    } leg_state,
        last_leg_state;

    float step_distance_multiplier = 100.0f;
    float max_spine_rotation = 0.25f;

    struct InterpolationSpeedMultiplier {
        float min = 0.02f, max = 0.08f;
    } interpolation_speed_multiplier;

    void set_new_splines(float walking_speed,
                         const std::list<BoxCollider>& colliders);

    void interpolate_splines(glm::vec2 dst[Spline::NUM_POINTS],
                             SplineIndex spline_index) const;

    friend Game;
};