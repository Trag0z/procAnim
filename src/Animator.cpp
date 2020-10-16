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
                               glm::vec2 move) {
    dst[P1] = src[P1] + move;
    dst[T1] = src[T1];
    dst[T2] = src[T2];
    dst[P2] = src[P2] + move;
}

// Takes a target_pos in model space and sets bone rotations so that the tail of
// bone[1] is at (or at the closest possible point to) target_pos.
static void solve_ik(Bone* const bones[2], glm::vec2 target_pos, bool is_leg) {
    SDL_assert(bones != nullptr);

    // This algorithm assumes that the angle between the bones in bind pose is
    // zero. Check this assumption here, just to be sure.
    SDL_assert(
        glm::length(glm::cross(
            bones[0]->bind_pose_transform[2] -
                bones[0]->bind_pose_transform * glm::vec3(bones[0]->tail, 1.0f),
            bones[1]->bind_pose_transform[2] -
                bones[1]->bind_pose_transform *
                    glm::vec3(bones[1]->tail, 1.0f))) < 0.1f);

    float target_distance = glm::length(bones[0]->head() - target_pos);

    if (target_distance > bones[0]->length + bones[1]->length) {
        glm::vec2 local_target_pos =
            glm::vec2(bones[0]->inverse_bind_pose_transform *
                      glm::inverse(bones[0]->parent->get_transform()) *
                      glm::vec3(target_pos, 1.0f));

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
                glm::vec2(b.inverse_bind_pose_transform *
                          glm::inverse(b.parent->get_transform()) *
                          glm::vec3(target_pos, 1.0f));
            local_target2[i] = local_target[i] * local_target[i];

            length2[i] = b.length * b.length;
        }

        float long_factor = (local_target2[0].x + local_target2[0].y -
                             length2[0] - length2[1]) /
                            (2 * bones[0]->length * bones[1]->length);

        bones[1]->rotation =
            atan2f(sqrtf(1.0f - long_factor * long_factor), long_factor);

        if (is_leg) {
            bones[1]->rotation *= -1.0f;
        }

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

const float Animator::MAX_SPINE_ROTATION = -0.25f * PI;

const Animator::WalkingSpeedMultiplier Animator::WALKING_SPEED_MULTIPLIER = {
    0.02f, 0.1f};

void Animator::init(const Player* parent_, RiggedMesh& mesh,
                    const std::list<BoxCollider>& colliders) {
    parent = parent_;
    spline_editor = new SplineEditor();
    spline_editor->init(parent, &spline_prototypes, limbs,
                        "../assets/player_splines.spl");

    GLuint indices[] = {0, 1, 2, 3};
    target_points_vao.init(indices, 4, nullptr, 4, GL_DYNAMIC_DRAW);

    leg_state = NEUTRAL;
    last_leg_state = NEUTRAL;

    interpolation_factor_between_splines = interpolation_factor_on_spline =
        0.0f;

    for (size_t i = 0; i < 4; ++i) {
        auto& limb = limbs[i];
        limb.spline.init(nullptr);

        glm::vec2 spline_points[Spline::NUM_POINTS];
        if (i == LEFT_ARM) {
            limb.bones[0] = mesh.find_bone("Arm_L_1");
            limb.bones[1] = mesh.find_bone("Arm_L_2");
        } else if (i == RIGHT_ARM) {
            limb.bones[0] = mesh.find_bone("Arm_R_1");
            limb.bones[1] = mesh.find_bone("Arm_R_2");
        } else if (i == LEFT_LEG) {
            limb.bones[0] = mesh.find_bone("Leg_L_1");
            limb.bones[1] = mesh.find_bone("Leg_L_2");
        } else {
            limb.bones[0] = mesh.find_bone("Leg_R_1");
            limb.bones[1] = mesh.find_bone("Leg_R_2");
        }
        limb.spline.set_points(spline_points);
    }
    spine = mesh.find_bone("Spine");
    pelvis_spline.init(spline_prototypes.idle[PELVIS].get_points());

    set_new_splines(0.0f, colliders);
}

void Animator::update(float delta_time, float walking_speed,
                      const MouseKeyboardInput& input,
                      const std::list<BoxCollider>& colliders) {
    // @CLEANUP
    if (arm_follows_mouse && input.mouse_button(MouseButton::LEFT)) {
        auto& right_arm = limbs[RIGHT_ARM];
        solve_ik(right_arm.bones,
                 parent->world_to_local_space(input.mouse_pos_world()), false);
        return;
    }

    // Update limbs
    float interpolation_speed = WALKING_SPEED_MULTIPLIER.MIN +
                                walking_speed * (WALKING_SPEED_MULTIPLIER.MAX -
                                                 WALKING_SPEED_MULTIPLIER.MIN);

    last_interpolation_factor_on_spline = interpolation_factor_on_spline;

    if (leg_state == NEUTRAL && last_leg_state != NEUTRAL) {
        // Transitioning to neutral, make this go extra quick
        interpolation_speed *= 3.0f;
    }

    interpolation_factor_on_spline = std::min(
        interpolation_factor_on_spline + delta_time * interpolation_speed,
        1.0f);

    for (size_t i = 0; i < 4; ++i) {
        auto& limb = limbs[i];

        glm::vec2 target_pos = parent->world_to_local_space(
            limb.spline.get_point_on_spline(interpolation_factor_on_spline));

        if (i == LEFT_LEG || i == RIGHT_LEG) {
            solve_ik(limb.bones, target_pos, true);
        } else {
            solve_ik(limb.bones, target_pos, false);
        }
    }

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
        if (leg_state == NEUTRAL) {
            // Start to walk
            last_leg_state = leg_state;
            leg_state = RIGHT_LEG_UP;
            interpolation_factor_between_splines = walking_speed;
            set_new_splines(walking_speed, colliders);

            last_interpolation_factor_on_spline =
                interpolation_factor_on_spline;
            interpolation_factor_on_spline = 0.4f;

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
            last_interpolation_factor_on_spline =
                interpolation_factor_on_spline;
            interpolation_factor_on_spline = 0.0f;
        }
    } else {
        // Player is standing
        if (leg_state != NEUTRAL) {
            last_leg_state = leg_state;
            leg_state = NEUTRAL;

            set_new_splines(walking_speed, colliders);

            last_interpolation_factor_on_spline =
                interpolation_factor_on_spline;
            interpolation_factor_on_spline = 0.0f;

        } else if (interpolation_factor_on_spline == 1.0f) {
            last_leg_state = NEUTRAL;
            set_new_splines(walking_speed, colliders);

            last_interpolation_factor_on_spline =
                interpolation_factor_on_spline;
            interpolation_factor_on_spline = 0.0f;
        }
    }
}

void Animator::render(const Renderer& renderer) {
    renderer.debug_shader.use();
    renderer.debug_shader.set_color(&Color::GREEN);

    // All the spline positions are in world space, so set the model matrix to
    // unity
    glm::mat3 model(1.0f);
    renderer.debug_shader.set_model(&model);

    // @CLEANUP: Calculate this less often
    DebugShader::Vertex target_points[4];
    for (size_t i = 0; i < 4; ++i) {
        target_points[i].pos = limbs[i].spline.get_point(P2);
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

glm::vec2 Animator::get_pelvis_pos() const {
    return pelvis_spline.get_point_on_spline(interpolation_factor_on_spline);
}

void Animator::set_new_splines(float walking_speed,
                               const std::list<BoxCollider>& colliders) {
    // Lean in walking direction when walking fast/running
    // TODO: Spine should interpolate to move smoothely
    spine->rotation = std::max(walking_speed - 0.2f, 0.0f) * MAX_SPINE_ROTATION;

    auto find_highest_ground_at = [&colliders,
                                   this](glm::vec2 local_pos) -> glm::vec2 {
        glm::vec2 world_pos = this->parent->local_to_world_space(local_pos);

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
        static bool moving_forward = true;

        if (interpolation_factor_on_spline == 1.0f) {
            moving_forward = !moving_forward;
        }

        if (last_leg_state != NEUTRAL) {
            moving_forward = false;
        }

        // Arms
        // @CLEANUP: Remove ? operators?
        Spline* prototype = moving_forward
                                ? &spline_prototypes.idle[ARM_FORWARD]
                                : &spline_prototypes.idle[ARM_BACKWARD];

        move_spline_points(spline_points, prototype->get_points(),
                           limbs[LEFT_ARM].origin());
        spline_to_world_space(spline_points);
        limbs[LEFT_ARM].spline.set_points(spline_points);

        move_spline_points(spline_points, prototype->get_points(),
                           limbs[RIGHT_ARM].origin());
        spline_to_world_space(spline_points);
        limbs[RIGHT_ARM].spline.set_points(spline_points);

        // Legs
        glm::vec2 ground_left =
            find_highest_ground_at(limbs[LEFT_LEG].origin());
        glm::vec2 ground_right =
            find_highest_ground_at(limbs[RIGHT_LEG].origin());

        set_spline_points(ground_left, glm::vec2(0.0f), glm::vec2(0.0f),
                          ground_left);
        limbs[LEFT_LEG].spline.set_points(spline_points);

        set_spline_points(ground_right, glm::vec2(0.0f), glm::vec2(0.0f),
                          ground_right);
        limbs[RIGHT_LEG].spline.set_points(spline_points);

        move_spline_points(spline_points,
                           spline_prototypes.idle[PELVIS].get_points(),
                           glm::vec2(0.0f, limbs[LEFT_LEG].length()));

        glm::vec2 pelvis_origin_world =
            ground_left + (ground_right - ground_left) * 0.5f;
        spline_to_world_space(spline_points);
        move_spline_points(spline_points, spline_points,
                           pelvis_origin_world - parent->get_position());

        if (!moving_forward) {
            set_spline_points(spline_points[3], spline_points[2],
                              spline_points[1], spline_points[0]);
        }

        pelvis_spline.set_points(spline_points);
    } else { // leg_state != NEUTRAL
        step_distance = walking_speed * STEP_DISTANCE_MULTIPLIER;

        interpolate_tangents(spline_points, LEG_FORWARD);
    }

    // else { // leg_state != NEUTRAL
    //         glm::vec2 interpolated_points_left[Spline::NUM_POINTS];
    //         glm::vec2 interpolated_points_right[Spline::NUM_POINTS];

    //         // Arms
    //         if (leg_state == LEFT_LEG_UP) {
    //             interpolate_splines(interpolated_points_left,
    //             ARM_BACKWARD);
    //             interpolate_splines(interpolated_points_right,
    //             ARM_FORWARD);
    //         } else {
    //             interpolate_splines(interpolated_points_left,
    //             ARM_FORWARD);
    //             interpolate_splines(interpolated_points_right,
    //             ARM_BACKWARD);
    //         }

    //         for (auto& point : interpolated_points_left) {
    //             point += limbs[LEFT_ARM].origin();
    //         }
    //         limbs[LEFT_ARM].spline.set_points(interpolated_points_left);

    //         for (auto& point : interpolated_points_right) {
    //             point += limbs[RIGHT_ARM].origin();
    //         }
    //         limbs[RIGHT_ARM].spline.set_points(interpolated_points_right);

    //         // Legs
    //         if (leg_state == LEFT_LEG_UP) {
    //             interpolate_splines(interpolated_points_left,
    //             LEG_FORWARD);
    //             interpolate_splines(interpolated_points_right,
    //             LEG_BACKWARD);
    //         } else {
    //             interpolate_splines(interpolated_points_left,
    //             LEG_BACKWARD);
    //             interpolate_splines(interpolated_points_right,
    //             LEG_FORWARD);
    //         }
    //         for (auto& point : interpolated_points_left) {
    //             point += limbs[LEFT_LEG].origin();
    //         }
    //         for (auto& point : interpolated_points_right) {
    //             point += limbs[RIGHT_LEG].origin();
    //         }

    //         // Always start the new spline at the current point of the
    //         last
    //         // spline
    //         glm::vec2 current_spline_point = last_foot_pos_left;
    //         interpolated_points_left[T1] +=
    //             current_spline_point -
    //             interpolated_points_left[P1];

    //         interpolated_points_left[P1] = current_spline_point;

    //         current_spline_point = last_foot_pos_right;
    //         interpolated_points_right[T1] +=
    //             current_spline_point -
    //             interpolated_points_right[P1];

    //         interpolated_points_right[P1] = current_spline_point;

    //         // Find a place to stand on as target for the front leg
    //         if (leg_state == LEFT_LEG_UP) {
    // #ifdef _DEBUG
    //             glm::vec2 ground =
    //                 find_highest_ground_at(interpolated_points_right[P1]);
    //             SDL_assert(std::abs(interpolated_points_right[P1].y
    //             -
    //                                 ground.y) < 0.1f);
    // #endif
    //             glm::vec2 body_movement_till_end_of_step =
    //                 limbs[RIGHT_LEG].spline.get_point_on_spline(
    //                     interpolation_factor_on_spline) -
    //                 interpolated_points_right[P2];

    //             glm::vec2 target_foot_pos =
    //                 find_highest_ground_at(interpolated_points_left[P2]
    //                 +
    //                                        body_movement_till_end_of_step)
    //                                        -
    //                 body_movement_till_end_of_step;

    //             float height_difference =
    //                 target_foot_pos.y -
    //                 interpolated_points_left[P2].y;
    //             if (height_difference < 0.0f) {

    //                 if (interpolated_points_right[P2].y -
    //                         height_difference <
    //                     limbs[RIGHT_LEG].origin().x +
    //                         limbs[RIGHT_LEG].length() * 0.5f) {

    //                     interpolated_points_right[T2].y -=
    //                         height_difference;
    //                     interpolated_points_right[P2].y -=
    //                         height_difference;
    //                 }
    //             } else {
    //                 interpolated_points_left[T2] +=
    //                     target_foot_pos -
    //                     interpolated_points_left[P2];
    //                 interpolated_points_left[P2] =
    //                 target_foot_pos;
    //             }

    //         } else { // leg_state == RIGHT_LEG_UP
    // #ifdef _DEBUG
    //             glm::vec2 ground =
    //                 find_highest_ground_at(interpolated_points_left[P1]);
    //             SDL_assert(std::abs(interpolated_points_left[P1].y
    //             -
    //                                 ground.y) < 0.1f);
    // #endif
    //             glm::vec2 body_movement_till_end_of_step =
    //                 limbs[LEFT_LEG].spline.get_point_on_spline(
    //                     interpolation_factor_on_spline) -
    //                 interpolated_points_left[P2];

    //             glm::vec2 target_foot_pos =
    //                 find_highest_ground_at(interpolated_points_right[P2]
    //                 +
    //                                        body_movement_till_end_of_step)
    //                                        -
    //                 body_movement_till_end_of_step;

    //             float height_difference =
    //                 target_foot_pos.y -
    //                 interpolated_points_right[P2].y;
    //             if (height_difference < 0.0f) {

    //                 if (interpolated_points_left[P2].y -
    //                 height_difference <
    //                     limbs[LEFT_LEG].origin().x +
    //                         limbs[LEFT_LEG].length() * 0.5f) {

    //                     interpolated_points_left[T2].y -=
    //                     height_difference;
    //                     interpolated_points_left[P2].y -=
    //                     height_difference;
    //                 }
    //             } else {
    //                 interpolated_points_right[T2] +=
    //                     target_foot_pos -
    //                     interpolated_points_right[P2];
    //                 interpolated_points_right[P2] =
    //                 target_foot_pos;
    //             }
    //         }

    //         limbs[LEFT_LEG].spline.set_points(interpolated_points_left);
    //         limbs[RIGHT_LEG].spline.set_points(interpolated_points_right);
    //     }
}

void Animator::interpolate_splines(glm::vec2 dst[Spline::NUM_POINTS],
                                   SplineIndex spline_index) const {
    for (size_t n_point = 0; n_point < Spline::NUM_POINTS; ++n_point) {
        auto point_name = static_cast<SplinePointName>(n_point);
        dst[n_point] =
            lerp(spline_prototypes.walk[spline_index].get_point(point_name),
                 spline_prototypes.run[spline_index].get_point(point_name),
                 interpolation_factor_between_splines);
    }
}

void Animator::interpolate_tangents(glm::vec2 dst[Spline::NUM_POINTS],
                                    SplineIndex spline_index) {
    dst[T1] = lerp(parent->local_to_world_space(
                       spline_prototypes.walk[spline_index].get_point(T1)),
                   parent->local_to_world_space(
                       spline_prototypes.run[spline_index].get_point(T1)),
                   interpolation_factor_between_splines);

    dst[T2] = lerp(parent->local_to_world_space(
                       spline_prototypes.walk[spline_index].get_point(T2)),
                   parent->local_to_world_space(
                       spline_prototypes.run[spline_index].get_point(T2)),
                   interpolation_factor_between_splines);
}