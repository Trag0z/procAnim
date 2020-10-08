#pragma once
#include "pch.h"
#include "Animator.h"
#include "Util.h"
#include "Mesh.h"
#include "Renderer.h"
#include "Player.h"

// Takes a target_pos in model space and finds two target_rotations for the
// bones, so that the tail of bone[1] is at (or at the closest possible point
// to) target_pos.
static void solve_ik(Bone* const bones[2],
                     const BoneRestrictions bone_restrictions[2],
                     glm::vec2 target_pos, float* target_rotations) {
    SDL_assert(bones != nullptr && bone_restrictions != nullptr);

    // TODO: find out why this is not pointing to the exact spot it should point
    // to

    // We need the target positions before the bone's rotation is applied to
    // find the new, absolute rotation. Therefore, we use the
    // inverse_bind_pose_transform
    glm::vec3 target_pos_bone_space[2];
    target_pos_bone_space[0] = bones[0]->inverse_bind_pose_transform *
                               glm::inverse(bones[0]->parent->get_transform()) *
                               glm::vec3(target_pos, 1.0f);

    target_pos_bone_space[1] = bones[1]->inverse_bind_pose_transform *
                               glm::inverse(bones[0]->get_transform()) *
                               glm::vec3(target_pos, 1.0f);

    float target_distance = glm::length(glm::vec3(target_pos, 1.0f) -
                                        bones[0]->get_transform() *
                                            bones[0]->bind_pose_transform[2]);

    // Find out if min_rotation or max_rotation is closer to the
    // target_rotations and set target_rotations to the closer one if it is out
    // of the restricted range
    // auto clamp_to_closest_restriction = [target_rotations,
    //                                      bone_restrictions](size_t
    //                                      bone_index) {
    //     auto rotation = target_rotations[bone_index];
    //     auto& restrictions = bone_restrictions[bone_index];

    //     SDL_assert(rotation > -2.0f * PI && rotation < 2.0f * PI);

    //     if (rotation > restrictions.min_rotation &&
    //         rotation < restrictions.max_rotation)
    //         return;

    //     float distance_to_min;
    //     if (rotation < restrictions.min_rotation - PI) {
    //         distance_to_min =
    //             std::abs(rotation + 2.0f * PI - restrictions.min_rotation);
    //         // SDL_TriggerBreakpoint();
    //     } else if (rotation > restrictions.min_rotation + PI) {
    //         distance_to_min =
    //             std::abs(rotation - 2.0f * PI - restrictions.min_rotation);
    //         // SDL_TriggerBreakpoint();
    //     } else {
    //         distance_to_min = std::abs(rotation - restrictions.min_rotation);
    //     }

    //     float distance_to_max;
    //     if (rotation < restrictions.max_rotation - PI) {
    //         distance_to_max =
    //             std::abs(rotation + 2.0f * PI - restrictions.max_rotation);
    //         // SDL_TriggerBreakpoint();
    //     } else if (rotation > restrictions.max_rotation + PI) {
    //         distance_to_max =
    //             std::abs(rotation - 2.0f * PI - restrictions.max_rotation);
    //         // SDL_TriggerBreakpoint();
    //     } else {
    //         distance_to_max = std::abs(rotation - restrictions.max_rotation);
    //     }

    //     if (distance_to_min < distance_to_max) {
    //         rotation = restrictions.min_rotation;
    //     } else {
    //         rotation = restrictions.max_rotation;
    //     }
    // };

    // NOTE: The resulting rotations here are applied counter clockwise, but
    // why?
    if (target_distance > bones[0]->length + bones[1]->length) {
        // Target out of reach
        // Get angle between local up (y-axis) and target position

        for (size_t i = 0; i < 2; ++i) {
            target_rotations[i] =
                atan2f(target_pos_bone_space[i].y, target_pos_bone_space[i].x) -
                degToRad(90.0f);

            SDL_assert(target_rotations[i] < 2.0f * PI &&
                       target_rotations[i] > -2.0f * PI);

            // clamp_to_closest_restriction(i);
        }
    } else {
        float target_distance2 = target_distance * target_distance;
        float length2[2] = {bones[0]->length * bones[0]->length,
                            bones[1]->length * bones[1]->length};

        float cosAngle0 = (target_distance2 + length2[0] - length2[1]) /
                          (2 * target_distance * bones[0]->length);

        float atan =
            atan2f(target_pos_bone_space[0].y, target_pos_bone_space[0].x);
        float acos = acosf(cosAngle0);
        target_rotations[0] = atan - acos - PI * 0.5f;

        float cosAngle1 = (length2[0] + length2[1] - target_distance2) /
                          (2.0f * bones[0]->length * bones[1]->length);
        float acos1 = acosf(cosAngle1);
        target_rotations[1] =
            PI -
            acos1; // BUG: PI should be the right value here, but then the arm
                   // overshoots the position by a bit. Find out why that is!
                   // 175 degrees gives almost pixel perfect results for arms.

        // If the target is unreachable, try rotating bone[0] the other way
        if ((target_rotations[1] < bone_restrictions[1].min_rotation ||
             target_rotations[1] > bone_restrictions[1].max_rotation)) {
            target_rotations[0] += acos * 2.0f;
            target_rotations[1] *= -1.0f;
        }

        // clamp_to_closest_restriction(0);
        // clamp_to_closest_restriction(1);
    }

    // NOTE: The following stuff should be covered by
    // clamp_to_closest_restriction()
    // If the target rotation is more than 180 degrees away from the current
    // rotation, choose the shorter way around the circle
    if (target_rotations[0] - bones[0]->rotation > PI) {
        target_rotations[0] -= 2.0f * PI;
    } else if (target_rotations[0] - bones[0]->rotation < -PI) {
        target_rotations[0] += 2.0f * PI;
    }

    if (target_rotations[1] - bones[1]->rotation > PI) {
        target_rotations[1] -= 2.0f * PI;
    } else if (target_rotations[1] - bones[1]->rotation < -PI) {
        target_rotations[1] += 2.0f * PI;
    }
}

glm::vec2 Limb::origin() const { return bones[0]->head(); }
float Limb::length() const { return bones[0]->length + bones[1]->length; }

//                          //
//          Animator        //
//                          //

const float Animator::MAX_SPINE_ROTATION = -0.25 * PI;

const Animator::WalkingSpeedMultiplier Animator::WALKING_SPEED_MULTIPLIER = {
    0.02f, 0.1f};

void Animator::init(const Player* parent_, RiggedMesh& mesh) {
    parent = parent_;
    spine = mesh.find_bone("Spine");
    spline_editor = new SplineEditor();
    spline_editor->init(parent, &spline_prototypes, limbs,
                        "../assets/player_splines.spl");

    GLuint indices[] = {0, 1, 2, 3};
    target_points_vao.init(indices, 4, nullptr, 4, GL_DYNAMIC_DRAW);

    leg_state = NEUTRAL;
    last_leg_state = RIGHT_LEG_UP;

    interpolation_factor_between_splines = interpolation_factor_on_spline =
        0.0f;

    last_ground_movement = glm::vec2(0.0f);

    for (size_t i = 0; i < 4; ++i) {
        auto& limb = limbs[i];
        limb.spline.init(nullptr);

        // @CLEANUP: are these already zeroed?
        limb.target_rotations[0] = 0.0f;
        limb.target_rotations[1] = 0.0f;

        limb.bone_restrictions[0] = {std::numeric_limits<float>::min(),
                                     std::numeric_limits<float>::max()};
        limb.bone_restrictions[1] = limb.bone_restrictions[0];

        if (i == LEFT_ARM || i == RIGHT_ARM) {
            limb.bone_restrictions[0] = {-0.5f * PI, 0.5 * PI};
            limb.bone_restrictions[1] = {0.0f, 0.75f * PI};
        } else {
            limb.bone_restrictions[0] = {0.0f, 0.0f}; //{-0.7f * PI, 0.7f * PI};
            limb.bone_restrictions[1] = {0.0f,
                                         0.0f}; //{degToRad(-120.0f), 0.0f};
        }

        if (i == LEFT_ARM) {
            limb.bones[0] = mesh.find_bone("Arm_L_1");
            limb.bones[1] = mesh.find_bone("Arm_L_2");
            set_limb_spline(static_cast<LimbIndex>(i), ARM_FORWARD);
        } else if (i == RIGHT_ARM) {
            limb.bones[0] = mesh.find_bone("Arm_R_1");
            limb.bones[1] = mesh.find_bone("Arm_R_2");
            set_limb_spline(static_cast<LimbIndex>(i), ARM_FORWARD);
        } else if (i == LEFT_LEG) {
            limb.bones[0] = mesh.find_bone("Leg_L_1");
            limb.bones[1] = mesh.find_bone("Leg_L_2");
            set_limb_spline(static_cast<LimbIndex>(i), LEG_FORWARD);
        } else {
            limb.bones[0] = mesh.find_bone("Leg_R_1");
            limb.bones[1] = mesh.find_bone("Leg_R_2");
            set_limb_spline(static_cast<LimbIndex>(i), LEG_FORWARD);
        }
    }
}

void Animator::update(float delta_time, float walking_speed,
                      const MouseKeyboardInput& input,
                      const std::list<BoxCollider>& colliders) {
    // @CLEANUP
    if (arm_follows_mouse && input.mouse_button_down(MouseButton::LEFT)) {
        auto& right_arm = limbs[RIGHT_ARM];
        solve_ik(right_arm.bones, right_arm.bone_restrictions,
                 parent->world_to_local_space(input.mouse_pos_world()),
                 right_arm.target_rotations);
        right_arm.bones[0]->rotation = right_arm.target_rotations[0];
        right_arm.bones[1]->rotation = right_arm.target_rotations[1];
    }

    // Update limbs
    float interpolation_speed = WALKING_SPEED_MULTIPLIER.MIN +
                                walking_speed * (WALKING_SPEED_MULTIPLIER.MAX -
                                                 WALKING_SPEED_MULTIPLIER.MIN);

    last_interpolation_factor_on_spline = interpolation_factor_on_spline;

    interpolation_factor_on_spline = std::min(
        interpolation_factor_on_spline + delta_time * interpolation_speed,
        1.0f);

    for (size_t i = 0; i < 4; ++i) {
        auto& limb = limbs[i];

        glm::vec2 target_pos =
            limb.spline.get_point_on_spline(interpolation_factor_on_spline);
        solve_ik(limb.bones, limb.bone_restrictions, target_pos,
                 limb.target_rotations);

        // @CLEANUP: This uses interpolation_factor_on_spline, which does not
        // really make sense. It probably works, but should be more
        // understandable.
        limb.bones[0]->rotation =
            lerp(limb.bones[0]->rotation, limb.target_rotations[0],
                 interpolation_factor_on_spline);
        limb.bones[1]->rotation =
            lerp(limb.bones[1]->rotation, limb.target_rotations[1],
                 interpolation_factor_on_spline);
    }

    // Player movement
    const Limb* grounded_limb;
    if (leg_state == LEFT_LEG_UP) {
        grounded_limb = &limbs[RIGHT_LEG];
    } else if (leg_state == RIGHT_LEG_UP) {
        grounded_limb = &limbs[LEFT_LEG];
    } else {
        // Leg state is NEUTRAL
        if (last_leg_state == LEFT_LEG_UP) {
            grounded_limb = &limbs[RIGHT_LEG];
        } else {
            SDL_assert(last_leg_state == RIGHT_LEG_UP);
            grounded_limb = &limbs[LEFT_LEG];
        }
    }
    last_ground_movement = grounded_limb->spline.get_point_on_spline(
                               interpolation_factor_on_spline) -
                           grounded_limb->spline.get_point_on_spline(
                               last_interpolation_factor_on_spline);

    /*
    NOTE:
    When calculating the target points for the players next
    step, there might be two very different results for the walking
    spline and the running spline (since the latter one steps way
    further). For example, if there are stairs in front of the player,
    the walk spline might find a target point in front of the first
    stair while the other finds a point on top of that first stair. I
    originally planned on interpolating between the two splines all the
    time, but then the foot would hit the front of the stair in the
    given example.

    A possible solution (that was used here): Whenever we need a new target
    point (i.e. when starting to walk or when a step is completed),
    interpolate between the splines and find the target points. These points
    are then locked in until the step is completed or the player stops
    moving mid step.
    */

    if (walking_speed > 0.0f) {
        // Lean in walking direction when walking fast/running
        spine->rotation =
            std::max(walking_speed - 0.2f, 0.0f) * MAX_SPINE_ROTATION;

        if (leg_state == NEUTRAL) {
            // Start to walk
            last_leg_state = leg_state;
            leg_state = RIGHT_LEG_UP;
            interpolation_factor_between_splines = walking_speed;
            last_interpolation_factor_on_spline =
                interpolation_factor_on_spline;
            interpolation_factor_on_spline = 0.4f;

            set_new_limb_splines(colliders);

        } else if (interpolation_factor_on_spline == 1.0f) {
            // Is walking and has reached the end of the current spline
            interpolation_factor_between_splines = walking_speed;
            last_interpolation_factor_on_spline =
                interpolation_factor_on_spline;
            interpolation_factor_on_spline = 0.0f;
            if (leg_state == LEFT_LEG_UP) {
                last_leg_state = leg_state;
                leg_state = RIGHT_LEG_UP;
                set_new_limb_splines(colliders);

            } else {
                last_leg_state = leg_state;
                leg_state = LEFT_LEG_UP;
                set_new_limb_splines(colliders);
            }
        }
    } else {
        // Player is standing
        if (leg_state != NEUTRAL) {
            last_interpolation_factor_on_spline =
                interpolation_factor_on_spline;
            interpolation_factor_on_spline = 0.0f;

            spine->rotation = 0.0f;
            last_leg_state = leg_state;
            leg_state = NEUTRAL;

            set_new_limb_splines(colliders);
        } else if (interpolation_factor_on_spline == 1.0f) {
            last_interpolation_factor_on_spline =
                interpolation_factor_on_spline;
            interpolation_factor_on_spline = 0.0f;

            set_new_limb_splines(colliders);
        }
    }
}

void Animator::render(const Renderer& renderer) {
    renderer.debug_shader.use();
    renderer.debug_shader.set_color(&Color::GREEN);

    // @CLEANUP: Calculate this less often
    DebugShader::Vertex target_points[4];
    for (size_t i = 0; i < 4; ++i) {
        target_points[i].pos = limbs[i].spline.point(Spline::P2);
    }
    target_points_vao.update_vertex_data(target_points, 4);
    target_points_vao.draw(GL_POINTS);

    if (renderer.draw_walk_splines) {
        limbs[LEFT_LEG].spline.render(renderer);
        limbs[RIGHT_LEG].spline.render(renderer);
    }
}

glm::vec2 Animator::get_tip_pos(LimbIndex limb_index) const {
    return limbs[limb_index].spline.get_point_on_spline(
        interpolation_factor_on_spline);
}

glm::vec2 Animator::get_last_ground_movement() const {
    return last_ground_movement;
}

glm::vec2 Animator::find_interpolated_target_point(
    SplineIndex spline_index) const noexcept {
    return lerp(spline_prototypes.walk[spline_index].point(Spline::P2),
                spline_prototypes.run[spline_index].point(Spline::P2),
                interpolation_factor_between_splines);
}

void Animator::set_limb_spline(LimbIndex limb_index, SplineIndex spline_index,
                               glm::vec2 target_pos_local_space) noexcept {
    auto& limb = limbs[limb_index];

    glm::vec2 spline_points[Spline::NUM_POINTS];

    if (leg_state == NEUTRAL) {
        for (size_t i = 0; i < Spline::NUM_POINTS; ++i) {
            Spline::PointName name = static_cast<Spline::PointName>(i);
            spline_points[name] =
                spline_prototypes.idle[spline_index].point(name) +
                limb.origin();
        }
    } else {
        for (size_t i = 0; i < Spline::NUM_POINTS; ++i) {
            Spline::PointName name = static_cast<Spline::PointName>(i);
            // NOTE: Maybe this interpolation doesn't "just work" and at least
            // T1 has to be calculated differently
            spline_points[name] =
                lerp(spline_prototypes.walk[spline_index].point(name),
                     spline_prototypes.run[spline_index].point(name),
                     interpolation_factor_between_splines);
            spline_points[name] += limb.origin();
        }
        // TODO: remove this and the default argument value
        target_pos_local_space = spline_points[Spline::P2];

        // T2 is in absolute local coordinates and has to be moved to be
        // relative to the target point to keep a good spline shape
        spline_points[Spline::T2] = spline_points[Spline::T2] -
                                    spline_points[Spline::P2] +
                                    target_pos_local_space;
        spline_points[Spline::P2] = target_pos_local_space;
    }

    limb.spline.set_points(spline_points);
}

void Animator::set_new_limb_splines(const std::list<BoxCollider>& colliders) {

    auto find_highest_ground_at = [&colliders,
                                   this](glm::vec2 pos) -> glm::vec2 {
        glm::vec2 world_pos = this->parent->local_to_world_space(pos);

        glm::vec2 result = glm::vec2(world_pos.x, 0.0f);
        for (auto& coll : colliders) {
            if (coll.left_edge() <= world_pos.x &&
                coll.right_edge() >= world_pos.x) {
                if (result.y == 0.0f || coll.top_edge() > result.y) {
                    result.y = coll.top_edge();
                }
            }
        }
        return this->parent->world_to_local_space(result);
    };

    if (leg_state == NEUTRAL) {
        static bool moving_forward = true;

        if (interpolation_factor_on_spline == 0.0f) {
            moving_forward = !moving_forward;
        }

        // Arms
        // @CLEANUP: Remove ? operators?
        Spline* prototype = moving_forward
                                ? &spline_prototypes.idle[ARM_FORWARD]
                                : &spline_prototypes.idle[ARM_BACKWARD];

        glm::vec2 spline_points[Spline::NUM_POINTS];
        for (size_t n_point = 0; n_point < Spline::NUM_POINTS; ++n_point) {
            spline_points[n_point] =
                prototype->point(static_cast<Spline::PointName>(n_point)) +
                limbs[LEFT_ARM].origin();
        }
        limbs[LEFT_ARM].spline.set_points(spline_points);

        for (size_t n_point = 0; n_point < Spline::NUM_POINTS; ++n_point) {
            spline_points[n_point] =
                prototype->point(static_cast<Spline::PointName>(n_point)) +
                limbs[RIGHT_ARM].origin();
        }
        limbs[RIGHT_ARM].spline.set_points(spline_points);

        // Legs
        prototype = moving_forward ? &spline_prototypes.idle[LEG_FORWARD]
                                   : &spline_prototypes.idle[LEG_BACKWARD];

        glm::vec2 spline_points_left[4];
        glm::vec2 spline_points_right[4];

        for (size_t n_point = 0; n_point < Spline::NUM_POINTS; ++n_point) {
            spline_points_left[n_point] =
                prototype->point(static_cast<Spline::PointName>(n_point)) +
                limbs[LEFT_LEG].origin();
            spline_points_right[n_point] =
                prototype->point(static_cast<Spline::PointName>(n_point)) +
                limbs[RIGHT_LEG].origin();
        }

        // Find out which foot stands on higher ground and move it's spline up
        // accordingly
        glm::vec2 ground_left =
            find_highest_ground_at(spline_points_left[Spline::P2]);
        glm::vec2 ground_right =
            find_highest_ground_at(spline_points_right[Spline::P2]);

        float height_difference = std::abs(ground_left.y - ground_right.y);

        SDL_assert(height_difference < limbs[LEFT_LEG].length());

        if (ground_left.y > ground_right.y) {
            for (auto& point : spline_points_left) {
                point.y += height_difference;
            }
        } else {
            for (auto& point : spline_points_right) {
                point.y += height_difference;
            }
        }

        // Always start the new spline at the current point of the last spline
        glm::vec2 current_spline_point_left =
            limbs[LEFT_LEG].spline.get_point_on_spline(
                last_interpolation_factor_on_spline);
        spline_points_left[Spline::T1] +=
            current_spline_point_left - spline_points_left[Spline::P1];

        spline_points_left[Spline::P1] = current_spline_point_left;

        glm::vec2 current_spline_point_right =
            limbs[RIGHT_LEG].spline.get_point_on_spline(
                last_interpolation_factor_on_spline);
        spline_points_right[Spline::T1] +=
            current_spline_point_right - spline_points_right[Spline::P1];

        spline_points_right[Spline::P1] = current_spline_point_right;

        if (last_leg_state == LEFT_LEG_UP) {
            glm::vec2 body_movement_till_end_of_step =
                current_spline_point_right - spline_points_right[Spline::P2];

            glm::vec2 target_foot_pos =
                find_highest_ground_at(spline_points_left[Spline::P2] +
                                       body_movement_till_end_of_step) -
                body_movement_till_end_of_step;

            spline_points_left[Spline::T2] +=
                target_foot_pos - spline_points_left[Spline::P2];
            spline_points_left[Spline::P2] = target_foot_pos;
        } else {
            glm::vec2 body_movement_till_end_of_step =
                current_spline_point_left - spline_points_left[Spline::P2];

            glm::vec2 target_foot_pos =
                find_highest_ground_at(spline_points_right[Spline::P2] +
                                       body_movement_till_end_of_step) -
                body_movement_till_end_of_step;

            spline_points_right[Spline::T2] +=
                target_foot_pos - spline_points_right[Spline::P2];
            spline_points_right[Spline::P2] = target_foot_pos;
        }
#ifdef _DEBUG
        if (last_leg_state == RIGHT_LEG_UP) {
            glm::vec2 ground =
                find_highest_ground_at(spline_points_left[Spline::P1]);
            SDL_assert(std::abs(spline_points_left[Spline::P1].y - ground.y) <
                       0.1f);
        }
        if (last_leg_state == LEFT_LEG_UP) {
            glm::vec2 ground =
                find_highest_ground_at(spline_points_right[Spline::P1]);
            SDL_assert(std::abs(spline_points_right[Spline::P1].y - ground.y) <
                       0.1f);
        }
#endif

        limbs[LEFT_LEG].spline.set_points(spline_points_left);
        limbs[RIGHT_LEG].spline.set_points(spline_points_right);

    } else {
        glm::vec2 interpolated_points_left[Spline::NUM_POINTS];
        glm::vec2 interpolated_points_right[Spline::NUM_POINTS];

        // Arms
        if (leg_state == LEFT_LEG_UP) {
            interpolate_splines(interpolated_points_left, ARM_BACKWARD);
            interpolate_splines(interpolated_points_right, ARM_FORWARD);
        } else {
            interpolate_splines(interpolated_points_left, ARM_FORWARD);
            interpolate_splines(interpolated_points_right, ARM_BACKWARD);
        }

        for (auto& point : interpolated_points_left) {
            point += limbs[LEFT_ARM].origin();
        }
        limbs[LEFT_ARM].spline.set_points(interpolated_points_left);

        for (auto& point : interpolated_points_right) {
            point += limbs[RIGHT_ARM].origin();
        }
        limbs[RIGHT_ARM].spline.set_points(interpolated_points_right);

        // Legs
        if (leg_state == LEFT_LEG_UP) {
            interpolate_splines(interpolated_points_left, LEG_FORWARD);
            interpolate_splines(interpolated_points_right, LEG_BACKWARD);
        } else {
            interpolate_splines(interpolated_points_left, LEG_BACKWARD);
            interpolate_splines(interpolated_points_right, LEG_FORWARD);
        }
        for (auto& point : interpolated_points_left) {
            point += limbs[LEFT_LEG].origin();
        }
        for (auto& point : interpolated_points_right) {
            point += limbs[RIGHT_LEG].origin();
        }

        // Always start the new spline at the current point of the last spline
        glm::vec2 current_spline_point =
            limbs[LEFT_LEG].spline.get_point_on_spline(
                last_interpolation_factor_on_spline);
        interpolated_points_left[Spline::T1] +=
            current_spline_point - interpolated_points_left[Spline::P1];

        interpolated_points_left[Spline::P1] = current_spline_point;

        current_spline_point = limbs[RIGHT_LEG].spline.get_point_on_spline(
            last_interpolation_factor_on_spline);
        interpolated_points_right[Spline::T1] +=
            current_spline_point - interpolated_points_right[Spline::P1];

        interpolated_points_right[Spline::P1] = current_spline_point;

        // Find a place to stand on as target for the front leg
        if (leg_state == LEFT_LEG_UP) {
#ifdef _DEBUG
            glm::vec2 ground =
                find_highest_ground_at(interpolated_points_right[Spline::P1]);
            SDL_assert(std::abs(interpolated_points_right[Spline::P1].y -
                                ground.y) < 0.1f);
#endif
            glm::vec2 body_movement_till_end_of_step =
                limbs[RIGHT_LEG].spline.get_point_on_spline(
                    last_interpolation_factor_on_spline) -
                interpolated_points_right[Spline::P2];

            glm::vec2 target_foot_pos =
                find_highest_ground_at(interpolated_points_left[Spline::P2] +
                                       body_movement_till_end_of_step) -
                body_movement_till_end_of_step;

#ifdef _DEBUG
            glm::vec2 origin = limbs[LEFT_LEG].origin();
            SDL_assert(glm::length(target_foot_pos - origin) <
                       limbs[LEFT_LEG].length());
#endif

            interpolated_points_left[Spline::T2] +=
                target_foot_pos - interpolated_points_left[Spline::P2];
            interpolated_points_left[Spline::P2] = target_foot_pos;
        } else {
#ifdef _DEBUG
            glm::vec2 ground =
                find_highest_ground_at(interpolated_points_left[Spline::P1]);
            SDL_assert(std::abs(interpolated_points_left[Spline::P1].y -
                                ground.y) < 0.1f);
#endif
            glm::vec2 body_movement_till_end_of_step =
                limbs[LEFT_LEG].spline.get_point_on_spline(
                    last_interpolation_factor_on_spline) -
                interpolated_points_left[Spline::P2];

            glm::vec2 target_foot_pos =
                find_highest_ground_at(interpolated_points_right[Spline::P2] +
                                       body_movement_till_end_of_step) -
                body_movement_till_end_of_step;
#ifdef _DEBUG
            glm::vec2 origin = limbs[RIGHT_LEG].origin();
            SDL_assert(glm::length(target_foot_pos - origin) <
                       limbs[RIGHT_LEG].length());
#endif
            interpolated_points_right[Spline::T2] +=
                target_foot_pos - interpolated_points_right[Spline::P2];
            interpolated_points_right[Spline::P2] = target_foot_pos;
        }

        limbs[LEFT_LEG].spline.set_points(interpolated_points_left);
        limbs[RIGHT_LEG].spline.set_points(interpolated_points_right);
    }
}

void Animator::interpolate_splines(glm::vec2 out_points[Spline::NUM_POINTS],
                                   SplineIndex spline_index) const {
    for (size_t n_point = 0; n_point < Spline::NUM_POINTS; ++n_point) {
        auto point_name = static_cast<Spline::PointName>(n_point);
        out_points[n_point] =
            lerp(spline_prototypes.walk[spline_index].point(point_name),
                 spline_prototypes.run[spline_index].point(point_name),
                 interpolation_factor_between_splines);
    }
}