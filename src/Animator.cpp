#pragma once
#include "pch.h"
#include "Animator.h"
#include "Util.h"
#include "Mesh.h"
#include "Renderer.h"
#include "Player.h"

// Moves all points in src by move and write them to dst. src and dst can point
// to the same array.
static void move_spline_points(glm::vec2* dst, const glm::vec2* src,
                               glm::vec2 move = glm::vec2(0.0f)) {
    dst[P1] = src[P1] + move;
    dst[T1] = src[T1];
    dst[T2] = src[T2];
    dst[P2] = src[P2] + move;
}

// Takes a target_pos in model space and sets bone rotations so that the tail of
// bone[1] is at (or at the closest possible point to) target_pos_model_space.
static void solve_ik(Bone* const bones[2], glm::vec2 target_pos_model_space) {
    SDL_assert(bones != nullptr);

    // This algorithm assumes that the angle between the bones in bind pose is
    // zero. Check this assumption here, just to be sure.
    SDL_assert(glm::length(glm::cross(
                   bones[0]->bind_pose_transform()[2] -
                       bones[0]->bind_pose_transform() *
                           glm::vec3(bones[0]->tail(), 1.0f),
                   bones[1]->bind_pose_transform()[2] -
                       bones[1]->bind_pose_transform() *
                           glm::vec3(bones[1]->tail(), 1.0f))) < 0.1f);

    float target_distance =
        glm::length(bones[0]->head() - target_pos_model_space);

    if (target_distance > bones[0]->length + bones[1]->length) {
        glm::vec2 local_target_pos =
            glm::vec2(bones[0]->inverse_bind_pose_transform() *
                      glm::inverse(bones[0]->parent()->transform()) *
                      glm::vec3(target_pos_model_space, 1.0f));

        bones[0]->rotation =
            atan2f(local_target_pos.y, local_target_pos.x) - PI * 0.5f;
        bones[1]->rotation = 0.0f;
    } else {
        // Target is in reach
        glm::vec2 local_target[2];
        glm::vec2 local_target2[2];
        float length2[2];
        for (size_t i = 0; i < 2; ++i) {
            Bone& b = *bones[i];

            local_target[i] =
                glm::vec2(b.inverse_bind_pose_transform() *
                          glm::inverse(b.parent()->transform()) *
                          glm::vec3(target_pos_model_space, 1.0f));
            local_target2[i] = local_target[i] * local_target[i];

            length2[i] = b.length * b.length;
        }

        float long_factor = (local_target2[0].x + local_target2[0].y -
                             length2[0] - length2[1]) /
                            (2 * bones[0]->length * bones[1]->length);

        long_factor = clamp(long_factor, -1.0f, 1.0f);

        bones[1]->rotation =
            atan2f(sqrtf(1.0f - long_factor * long_factor), long_factor);
        SDL_assert(bones[1]->rotation >= 0.0f);

        bones[1]->rotation *= -1.0f;

        float gamma = atan2f(bones[1]->length * sinf(bones[1]->rotation),
                             bones[0]->length +
                                 bones[1]->length * cosf(bones[1]->rotation));

        bones[0]->rotation =
            atan2f(local_target[0].y, local_target[0].x) - gamma;

        bones[0]->rotation -= 0.5f * PI;
    }
}

glm::vec2 Limb::origin() const { return bones[0]->head(); }

float Limb::length() const { return bones[0]->length + bones[1]->length; }

//                          //
//          Animator        //
//                          //

void Animator::init(const Player* parent_, RiggedMesh& mesh,
                    const std::list<BoxCollider>& colliders) {
    parent = parent_;
    spline_editor = new SplineEditor();
    spline_editor->init(parent, &spline_prototypes, limbs,
                        "../assets/player_splines.spl");

    leg_state = NEUTRAL;
    last_leg_state = NEUTRAL;

    interpolation_factor_between_splines = interpolation_factor_on_spline =
        0.0f;

    limbs[LEFT_LEG].spline.init(nullptr);
    limbs[LEFT_LEG].bones[0] = mesh.find_bone("Leg_L_1");
    limbs[LEFT_LEG].bones[1] = mesh.find_bone("Leg_L_2");

    limbs[RIGHT_LEG].spline.init(nullptr);
    limbs[RIGHT_LEG].bones[0] = mesh.find_bone("Leg_R_1");
    limbs[RIGHT_LEG].bones[1] = mesh.find_bone("Leg_R_2");

    weapon = mesh.find_bone("Weapon");

    set_new_splines(0.0f, colliders);
    interpolation_factor_on_spline =
        1.0f; // Make the palyer move to the final position of the initial
              // spline instantly
}

void Animator::update(float delta_time, float walking_speed,
                      glm::vec2 right_stick_input,
                      const std::list<BoxCollider>& colliders) {
    // Weapon animation
    if (!parent->is_facing_right()) {
        right_stick_input.x *= -1.0f;
    }

    float weapon_rotation =
        atan2f(right_stick_input.y, right_stick_input.x) - PI * 0.5f;

    float weapon_length = glm::length(right_stick_input) * max_weapon_length;

    weapon->rotation = weapon_rotation;
    SDL_assert(weapon->tail().x == 0.0f);
    weapon->length = weapon_length;

    // Walk animation
    if (walking_speed > 0.0f) {
        if (leg_state == NEUTRAL) {
            // Start to walk
            last_leg_state = leg_state;
            leg_state = RIGHT_LEG_UP;
            interpolation_factor_between_splines = walking_speed;
            set_new_splines(walking_speed, colliders);

        } else if (interpolation_factor_on_spline == 1.0f) {
            // Is walking and has reached the end of the current spline
            interpolation_factor_between_splines = walking_speed;
            if (leg_state == LEFT_LEG_UP) {
                last_leg_state = leg_state;
                leg_state = RIGHT_LEG_UP;
                set_new_splines(walking_speed, colliders);

            } else {
                last_leg_state = leg_state;
                leg_state = LEFT_LEG_UP;
                set_new_splines(walking_speed, colliders);
            }
        }
    } else {
        // Player is standing
        if (leg_state != NEUTRAL) {
            last_leg_state = leg_state;
            leg_state = NEUTRAL;

            set_new_splines(walking_speed, colliders);

        } else if (interpolation_factor_on_spline == 1.0f) {
            last_leg_state = NEUTRAL;
            set_new_splines(walking_speed, colliders);
        }
    }

    // Update legs
    float interpolation_speed =
        interpolation_speed_multiplier.min +
        walking_speed * (interpolation_speed_multiplier.max -
                         interpolation_speed_multiplier.min);

    if (leg_state == NEUTRAL && last_leg_state != NEUTRAL) {
        // Transitioning to neutral, make this go extra quick
        interpolation_speed *= 3.0f;
    }

    interpolation_factor_on_spline = std::min(
        interpolation_factor_on_spline + delta_time * interpolation_speed,
        1.0f);

    for (size_t n_limb = 0; n_limb < 2; ++n_limb) {
        auto& limb = limbs[n_limb];

        glm::vec2 target_pos = parent->world_to_local_space(
            limb.spline.get_point_on_spline(interpolation_factor_on_spline));

        solve_ik(limb.bones, target_pos);
    }
}

void Animator::render(const Renderer& renderer) {
    renderer.debug_shader.use();
    renderer.debug_shader.set_color(&Color::GREEN);

    // All the spline positions are already in world space, so set the model
    // matrix to unity
    glm::mat3 model(1.0f);
    renderer.debug_shader.set_model(&model);

    if (renderer.draw_leg_splines) {
        limbs[LEFT_LEG].spline.render(renderer, true);
        limbs[RIGHT_LEG].spline.render(renderer, true);
    }
}

glm::vec2 Animator::tip_pos(LegIndex limb_index) const {
    return limbs[limb_index].spline.get_point_on_spline(
        interpolation_factor_on_spline);
}

void Animator::set_new_splines(float walking_speed,
                               const std::list<BoxCollider>& colliders) {

    auto find_highest_ground_at =
        [&colliders](glm::vec2 world_pos) -> glm::vec2 {
        glm::vec2 result = glm::vec2(world_pos.x, 0.0f);
        for (auto& coll : colliders) {
            if (coll.left_edge() <= world_pos.x &&
                coll.right_edge() >= world_pos.x) {
                if (result.y == 0.0f || coll.top_edge() > result.y) {
                    result.y = coll.top_edge();
                }
            }
        }
        return result;
    };

    auto spline_to_world_space = [this](glm::vec2 spline[Spline::NUM_POINTS]) {
        spline[P1] = parent->local_to_world_space(spline[P1]);
        spline[P2] = parent->local_to_world_space(spline[P2]);

        spline[T1] = parent->local_to_world_scale(spline[T1]);
        spline[T2] = parent->local_to_world_scale(spline[T2]);
    };

    glm::vec2 spline_points[Spline::NUM_POINTS];
    auto set_spline_points = [&spline_points](glm::vec2 p1 = glm::vec2(0.0f),
                                              glm::vec2 t1 = glm::vec2(0.0f),
                                              glm::vec2 t2 = glm::vec2(0.0f),
                                              glm::vec2 p2 = glm::vec2(0.0f)) {
        spline_points[P1] = p1;
        spline_points[T1] = t1;
        spline_points[T2] = t2;
        spline_points[P2] = p2;
    };

    if (leg_state == NEUTRAL) {
        static bool moving_forward =
            true; // Indicates whether the idle spline should be traversed
                  // forwards or backwards

        if (interpolation_factor_on_spline == 1.0f) {
            moving_forward = !moving_forward;
        }

        if (last_leg_state != NEUTRAL) {
            // Character just stopped walking
            moving_forward = false;
        }

        // Legs
        glm::vec2 ground_left = find_highest_ground_at(
            parent->local_to_world_space(limbs[LEFT_LEG].origin()));
        glm::vec2 ground_right = find_highest_ground_at(
            parent->local_to_world_space(limbs[RIGHT_LEG].origin()));

        set_spline_points(limbs[LEFT_LEG].spline.get_point_on_spline(
                              interpolation_factor_on_spline),
                          glm::vec2(0.0f), glm::vec2(0.0f), ground_left);
        limbs[LEFT_LEG].spline.set_points(spline_points);

        set_spline_points(limbs[RIGHT_LEG].spline.get_point_on_spline(
                              interpolation_factor_on_spline),
                          glm::vec2(0.0f), glm::vec2(0.0f), ground_right);
        limbs[RIGHT_LEG].spline.set_points(spline_points);

    } else { // leg_state != NEUTRAL
        step_distance_world = walking_speed * step_distance_multiplier;

        if (!parent->is_facing_right()) {
            step_distance_world *= -1.0f;
        }

        if (last_leg_state == NEUTRAL) {
            step_distance_world *= 0.5f;
        }

        // Limbs
        LegIndex forward_leg, backward_leg;
        if (leg_state == RIGHT_LEG_UP) {
            forward_leg = RIGHT_LEG;
            backward_leg = LEFT_LEG;
        } else {
            forward_leg = LEFT_LEG;
            backward_leg = RIGHT_LEG;
        }

        // Legs
        interpolate_splines(spline_points, LEG_FORWARD);
        move_spline_points(spline_points, spline_points,
                           limbs[forward_leg].origin());
        spline_to_world_space(spline_points);

        spline_points[P1] = limbs[forward_leg].spline.get_point_on_spline(
            interpolation_factor_on_spline);
        spline_points[P1] = find_highest_ground_at(spline_points[P1]);

        spline_points[P2].x =
            parent->local_to_world_space(limbs[forward_leg].origin()).x +
            step_distance_world *
                1.5f; // Moved by 1.5 times the step distance so the target
                      // position is half a step in front of the body _after_
                      // the body has moved one step distance
        glm::vec2 ground = find_highest_ground_at(spline_points[P2]);
        spline_points[P2].y = ground.y;

        limbs[forward_leg].spline.set_points(spline_points);

        glm::vec2 backward_leg_target_point =
            limbs[backward_leg].spline.point(P2);

        set_spline_points(backward_leg_target_point, glm::vec2(0.0f),
                          glm::vec2(0.0f), backward_leg_target_point);
        limbs[backward_leg].spline.set_points(spline_points);
    }

    interpolation_factor_on_spline = 0.0f;
}

void Animator::interpolate_splines(glm::vec2 dst[Spline::NUM_POINTS],
                                   SplineIndex spline_index) const {
    for (size_t n_point = 0; n_point < Spline::NUM_POINTS; ++n_point) {
        auto point_name = static_cast<SplinePointName>(n_point);
        dst[n_point] =
            lerp(spline_prototypes.walk[spline_index].point(point_name),
                 spline_prototypes.run[spline_index].point(point_name),
                 interpolation_factor_between_splines);
    }
}