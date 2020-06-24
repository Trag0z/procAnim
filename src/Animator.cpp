#pragma once
#include "pch.h"
#include "Animator.h"
#include "Util.h"
#include "Mesh.h"
#include "Game.h"

// Takes a target_pos in model space and finds two target_rotations for the
// bones, so that the tail of bone[1] is at (or at the closest possible point
// to) target_pos
static void resolve_ik(Bone* const bones[2],
                       const BoneRestrictions bone_restrictions[2],
                       glm::vec4 target_pos, float* target_rotations) {
    SDL_assert(bones != nullptr && bone_restrictions != nullptr);

    // We need the target positions before the bone's rotation is applied to
    // find the new, absolute rotation. Therefore, we use the
    // inverse_bind_pose_transform
    glm::vec4 target_pos_bone_space[2];
    target_pos_bone_space[0] = bones[0]->inverse_bind_pose_transform *
                               glm::inverse(bones[0]->parent->get_transform()) *
                               target_pos;

    target_pos_bone_space[1] = bones[1]->inverse_bind_pose_transform *
                               glm::inverse(bones[0]->get_transform()) *
                               target_pos;

    float target_distance = glm::length(
        static_cast<glm::vec3>(target_pos - bones[0]->bind_pose_transform[3]));

    // Find out if min_rotation or max_rotation is closer to the
    // target_rotations and set target_rotations to the closer one if it is out
    // of the restricted range
    auto clamp_to_closest_restriction = [target_rotations,
                                         bone_restrictions](size_t bone_index) {
        if (target_rotations[bone_index] < 0.0f) {
            if (std::abs(target_rotations[bone_index] -
                         bone_restrictions[bone_index].min_rotation) >
                std::abs(target_rotations[bone_index] + 2.0f * PI -
                         bone_restrictions[bone_index].max_rotation)) {
                target_rotations[bone_index] += 2.0f * PI;
            }
        } else {
            if (std::abs(target_rotations[bone_index] - 2.0f * PI -
                         bone_restrictions[bone_index].min_rotation) <
                std::abs(target_rotations[bone_index] -
                         bone_restrictions[bone_index].max_rotation)) {
                target_rotations[bone_index] -= 2.0f * PI;
            }
        }

        target_rotations[bone_index] = // Does this actually change the value?
            clamp(target_rotations[bone_index],
                  bone_restrictions[bone_index].min_rotation,
                  bone_restrictions[bone_index].max_rotation);
    };

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
    /* // If the target rotation is more than 180 degrees away from the current
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
    } */
}

LimbAnimator::LimbAnimator(Bone* b1, Bone* b2, Spline* s,
                           BoneRestrictions restrictions[2]) {
    bones[0] = b1;
    bones[1] = b2;
    splines = s;

    if (restrictions == nullptr) {
        bone_restrictions[0] = {std::numeric_limits<float>::min(),
                                std::numeric_limits<float>::max()};
        bone_restrictions[1] = bone_restrictions[0];
    } else {
        bone_restrictions[0] = restrictions[0];
        bone_restrictions[1] = restrictions[1];
    }

    tip_pos = bones[1]->get_transform() * bones[1]->bind_pose_transform *
              bones[1]->tail;
    last_tip_movement = glm::vec4(0.0f);

    // Init render data for rendering target_pos as a point
    GLuint index = 0;
    target_point_vao.init(&index, 1, NULL, 1);
}

void LimbAnimator::update(float delta_time, float movement_speed) {
    lerp_interpolation_factor =
        std::min(lerp_interpolation_factor + delta_time * movement_speed, 1.0f);

    if (animation_state == NEUTRAL) {
        // @CLEANUP: Maybe save the default bone position somewhere?

        target_pos = bones[1]->get_transform() * bones[1]->bind_pose_transform *
                     bones[1]->tail;
        target_rotations[0] = 0.0f;
        target_rotations[1] = 0.0f;

    } else {
        spline_interpolation_factor = std::min(
            spline_interpolation_factor + delta_time * movement_speed, 1.0f);

        target_pos = splines[animation_state].get_point_on_spline(
            spline_interpolation_factor);

        resolve_ik(bones, bone_restrictions, target_pos, target_rotations);
    }
    bones[0]->rotation = lerp(bones[0]->rotation, target_rotations[0],
                              lerp_interpolation_factor);
    bones[1]->rotation = lerp(bones[1]->rotation, target_rotations[1],
                              lerp_interpolation_factor);

    // Update tip_pos
    glm::vec4 new_tip_pos = bones[1]->get_transform() *
                            bones[1]->bind_pose_transform * bones[1]->tail;
    last_tip_movement = new_tip_pos - tip_pos;
    tip_pos = new_tip_pos;
}

void WalkAnimator::init(const Entity* parent, RiggedMesh& mesh) {
    for (auto& s : splines) {
        s.init();
    }

    BoneRestrictions restrictions[2] = {{-0.5f * PI, 0.5 * PI},
                                        {0.0f, 0.75f * PI}};
    limb_animators[LEFT_ARM] =
        LimbAnimator(mesh.find_bone("Arm_L_1"), mesh.find_bone("Arm_L_2"),
                     &splines[0], restrictions);
    limb_animators[RIGHT_ARM] =
        LimbAnimator(mesh.find_bone("Arm_R_1"), mesh.find_bone("Arm_R_2"),
                     &splines[2], restrictions);

    restrictions[0] = {0.0f, 0.0f}; //{-0.7f * PI, 0.7f * PI};
    restrictions[1] = {0.0f, 0.0f}; //{degToRad(-120.0f), 0.0f};
    limb_animators[LEFT_LEG] =
        LimbAnimator(mesh.find_bone("Leg_L_1"), mesh.find_bone("Leg_L_2"),
                     &splines[4], restrictions);
    limb_animators[RIGHT_LEG] =
        LimbAnimator(mesh.find_bone("Leg_R_1"), mesh.find_bone("Leg_R_2"),
                     &splines[6], restrictions);

    std::string limb_names[8] = {"Left arm forward",  "Left arm backward",
                                 "Right arm forward", "Right arm backward",
                                 "Left leg forward",  "Left leg backward",
                                 "Right leg forward", "Right leg backward"};

    const Bone* limb_bones[4][2];
    for (size_t i = 0; i < 4; ++i) {
        limb_bones[i][0] = limb_animators[i].bones[0];
        limb_bones[i][1] = limb_animators[i].bones[1];
    }

    spline_editor.init(parent, splines, 8, limb_names, limb_bones);
    // spline_editor.init(parent, splines, "../assets/player_splines.spl");
}

void WalkAnimator::update(float delta_time, float walking_speed,
                          AnimState state) {

    auto set_interpolations = [this](float spline_interpolation,
                                     float lerp_interpolation) -> void {
        for (auto& anim : limb_animators) {
            anim.spline_interpolation_factor = spline_interpolation;
            anim.lerp_interpolation_factor = lerp_interpolation;
        }
    };

    if (state == AnimState::WALKING) {
        if (leg_state == NEUTRAL) {
            // Starting to walk
            leg_state = RIGHT_LEG_UP;
            limb_animators[LEFT_ARM].animation_state =
                LimbAnimator::MOVE_FORWARD;
            limb_animators[RIGHT_ARM].animation_state =
                LimbAnimator::MOVE_BACKWARD;
            limb_animators[LEFT_LEG].animation_state =
                LimbAnimator::MOVE_BACKWARD;
            limb_animators[RIGHT_LEG].animation_state =
                LimbAnimator::MOVE_FORWARD;
            grounded_leg_index = LEFT_LEG;
            set_interpolations(0.4f, 0.0f);
        } else if (limb_animators[LEFT_LEG].spline_interpolation_factor ==
                       1.0f &&
                   limb_animators[RIGHT_LEG].spline_interpolation_factor ==
                       1.0f) { // @CLEANUP: They both move at the same speed, so
                               // one check should be enough
            // Is walking and has reached the end of the current spline
            if (leg_state == RIGHT_LEG_UP) {
                leg_state = LEFT_LEG_UP;
                limb_animators[LEFT_ARM].animation_state =
                    LimbAnimator::MOVE_BACKWARD;
                limb_animators[RIGHT_ARM].animation_state =
                    LimbAnimator::MOVE_FORWARD;
                limb_animators[LEFT_LEG].animation_state =
                    LimbAnimator::MOVE_FORWARD;
                limb_animators[RIGHT_LEG].animation_state =
                    LimbAnimator::MOVE_BACKWARD;
                grounded_leg_index = RIGHT_LEG;
                set_interpolations(0.0f, 0.0f);
            } else {
                leg_state = RIGHT_LEG_UP;
                limb_animators[LEFT_ARM].animation_state =
                    LimbAnimator::MOVE_FORWARD;
                limb_animators[RIGHT_ARM].animation_state =
                    LimbAnimator::MOVE_BACKWARD;
                limb_animators[LEFT_LEG].animation_state =
                    LimbAnimator::MOVE_BACKWARD;
                limb_animators[RIGHT_LEG].animation_state =
                    LimbAnimator::MOVE_FORWARD;
                grounded_leg_index = LEFT_LEG;
                set_interpolations(0.0f, 0.0f);
            }
        }
    } else { // Player is standing
        if (leg_state != NEUTRAL) {
            leg_state = NEUTRAL;
            for (auto& anim : limb_animators) {
                anim.animation_state = LimbAnimator::NEUTRAL;
            }
            set_interpolations(0.0f, 0.0f);
        }
    }

    for (auto& anim : limb_animators) {
        anim.update(delta_time, walking_speed);
    }
}

void WalkAnimator::render(const Renderer& renderer) {
    renderer.debug_shader.use();
    renderer.debug_shader.set_color(&Colors::GREEN);

    for (auto& anim : limb_animators) {
        // @CLEANUP: Maybe remove this?
        if (anim.target_pos.w == 0.0f) {
            // target_pos has not yet been set, so there's nothing to do
            continue;
        }

        anim.target_point_vao.update_vertex_data(
            reinterpret_cast<DebugShader::Vertex*>(&anim.target_pos),
            1); // ugly, but it works

        anim.target_point_vao.draw(GL_POINTS);
    }
}