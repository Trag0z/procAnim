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

const float ArmAnimator::animation_speed = 0.1f;

ArmAnimator::ArmAnimator(Bone* b1, Bone* b2, BoneRestrictions restrictions[2]) {
    bones[0] = b1;
    bones[1] = b2;

    if (restrictions == nullptr) {
        bone_restrictions[0] = {-2.0f * PI, 2.0f * PI};
        bone_restrictions[1] = bone_restrictions[0];
    } else {
        bone_restrictions[0] = restrictions[0];
        bone_restrictions[1] = restrictions[1];
    }

    target_pos = glm::vec4{0.0f, 0.0f, 0.0f, 0.0f};

    // Init render data for rendering target_pos_model_space as a point
    GLuint index = 0;

    vao.init(&index, 1, NULL, 1);
}

void ArmAnimator::update(float delta_time) {
    // Return if no target position was set
    if (glm::length(target_pos) == 0.0f) {
        return;
    }

    float target_rotations[2];
    resolve_ik(bones, bone_restrictions, target_pos, target_rotations);

    bones[0]->rotation = lerp(bones[0]->rotation, target_rotations[0],
                              std::min(1.0f, animation_speed * delta_time));

    bones[1]->rotation = lerp(bones[1]->rotation, target_rotations[1],
                              std::min(1.0f, animation_speed * delta_time));
}

LegAnimator::LegAnimator(Bone* b1, Bone* b2, BoneRestrictions restrictions[2]) {
    bones[0] = b1;
    bones[1] = b2;

    if (restrictions == nullptr) {
        bone_restrictions[0] = {std::numeric_limits<float>::min(),
                                std::numeric_limits<float>::max()};
        bone_restrictions[1] = bone_restrictions[0];
    } else {
        bone_restrictions[0] = restrictions[0];
        bone_restrictions[1] = restrictions[1];
    }

    foot_pos = bones[1]->get_transform() * bones[1]->bind_pose_transform *
               bones[1]->tail;
    last_foot_movement = glm::vec4(0.0f);
    grounded = false;

    // Init render data for rendering target_pos as a point
    GLuint index = 0;
    target_point_vao.init(&index, 1, NULL, 1);

    GLuint circle_indices[circle_segments];
    DebugShaderVertex circle_vertices[circle_segments];
    float radius = bones[0]->length + bones[1]->length;
    // glm::mat4 bone_transform = bones[0]->inverse_bind_pose_transform;

    for (size_t i = 0; i < circle_segments; ++i) {
        circle_indices[i] = static_cast<GLuint>(i);

        float theta = static_cast<float>(i) /
                      static_cast<float>(circle_segments) * 2.0f * PI;

        circle_vertices[i].pos =
            // bone_transform *
            glm::vec4(radius * cosf(theta), radius * sinf(theta), 0.0f, 1.0f);
    }

    circle_vao.init(circle_indices, circle_segments, circle_vertices,
                    circle_segments);
}

void LegAnimator::update(float delta_time, float walking_speed) {
    // If no target position was set, update foot_pos and return
    if (spline == nullptr) {
        // @CLEANUP: Maybe save the default bone position somewhere?
        bones[0]->rotation = 0.0f;
        bones[1]->rotation = 0.0f;
        target_pos = bones[1]->get_transform() * bones[1]->bind_pose_transform *
                     bones[1]->tail;
        resolve_ik(bones, bone_restrictions, target_pos, target_rotations);
        // NOTE: delta_time is always 1.0f if we hit the target framerate and
        // the game is running at normal speed multiplier

        // NOTE: These lerps never reach 1.0f because walking_speed * delta_time
        // is always about 0.2
        bones[0]->rotation = lerp(bones[0]->rotation, target_rotations[0],
                                  walking_speed * delta_time * 10.0f);

        bones[1]->rotation = lerp(bones[1]->rotation, target_rotations[1],
                                  walking_speed * delta_time * 10.0f);
    } else {
        current_interpolation =
            std::min(current_interpolation + delta_time * walking_speed, 1.0f);
        target_pos = spline->get_point_on_spline(current_interpolation);
        target_pos.x + bones[0]
                           ->inverse_bind_pose_transform[3]
                           .x; // Move left/right to be centered under directly
                               // under bone[0]

        resolve_ik(bones, bone_restrictions, target_pos, target_rotations);

        bones[0]->rotation = target_rotations[0];
        bones[1]->rotation = target_rotations[1];
    }

    // Update foot_pos
    glm::vec4 new_foot_pos = bones[1]->get_transform() *
                             bones[1]->bind_pose_transform * bones[1]->tail;
    last_foot_movement = new_foot_pos - foot_pos;
    foot_pos = new_foot_pos;
}

void WalkAnimator::init(const Entity* parent, RiggedMesh& mesh) {
    BoneRestrictions restrictions[2] = {{-0.5f * PI, 0.5 * PI},
                                        {0.0f, 0.75f * PI}};
    arm_animators[0] = ArmAnimator(mesh.find_bone("Arm_L_1"),
                                   mesh.find_bone("Arm_L_2"), restrictions);
    arm_animators[1] = ArmAnimator(mesh.find_bone("Arm_R_1"),
                                   mesh.find_bone("Arm_R_2"), restrictions);

    restrictions[0] = {0.0f, 0.0f}; //{-0.7f * PI, 0.7f * PI};
    restrictions[1] = {0.0f, 0.0f}; //{degToRad(-120.0f), 0.0f};
    leg_animators[0] = LegAnimator(mesh.find_bone("Leg_L_1"),
                                   mesh.find_bone("Leg_L_2"), restrictions);
    leg_animators[1] = LegAnimator(mesh.find_bone("Leg_R_1"),
                                   mesh.find_bone("Leg_R_2"), restrictions);

    splines[0].init();
    splines[1].init();
    splines[2].init();
    splines[3].init();

    std::string names[] = {
        {"Leg_Forward"}, {"Leg_Backward"}, {"Arm_Forward"}, {"Arm_Backward"}};

    spline_editor.init(parent, splines, "../assets/player_splines.spl");
}

void WalkAnimator::update(float delta_time, float walking_speed,
                          AnimState state) {

    auto reset_interpolations = [this]() -> void {
        leg_animators[0].current_interpolation = 0.0f;
        leg_animators[1].current_interpolation = 0.0f;
    };

    if (state == AnimState::WALKING) {
        if (leg_state == NEUTRAL) {
            // Starting to walk
            leg_state = RIGHT_LEG_UP;
            leg_animators[LEFT_LEG].spline = &splines[LEG_BACKWARD];
            leg_animators[RIGHT_LEG].spline = &splines[LEG_FORWARD];
            reset_interpolations();
        } else if (leg_animators[0].current_interpolation == 1.0f &&
                   leg_animators[1].current_interpolation == 1.0f) {
            // Is walking and has reached the end of the current spline
            if (leg_state == RIGHT_LEG_UP) {
                leg_state = LEFT_LEG_UP;
                leg_animators[LEFT_LEG].spline = &splines[LEG_FORWARD];
                leg_animators[RIGHT_LEG].spline = &splines[LEG_BACKWARD];
                reset_interpolations();
            } else {
                leg_state = RIGHT_LEG_UP;
                leg_animators[LEFT_LEG].spline = &splines[LEG_BACKWARD];
                leg_animators[RIGHT_LEG].spline = &splines[LEG_FORWARD];
                reset_interpolations();
            }
        }
    } else { // Player is standing
        if (leg_state != NEUTRAL) {
            leg_state = NEUTRAL;
            leg_animators[LEFT_LEG].spline = nullptr;
            leg_animators[RIGHT_LEG].spline = nullptr;
        }
    }

    leg_animators[0].update(delta_time, walking_speed);
    leg_animators[1].update(delta_time, walking_speed);
    arm_animators[0].update(delta_time);
    arm_animators[1].update(delta_time);
}

void WalkAnimator::render(const RenderData& render_data) {
    glUseProgram(render_data.debug_shader.id);
    glUniform4f(render_data.debug_shader.color_loc, 0.0f, 1.0f, 0.0f, 1.0f);

    for (auto& anim : arm_animators) {
        if (anim.target_pos.w == 0.0f) {
            // target_pos has not yet been set, so there's nothing to do
            continue;
        }

        anim.vao.update_vertex_data(
            reinterpret_cast<DebugShaderVertex*>(&anim.target_pos),
            1); // ugly, but it works

        anim.vao.draw(GL_POINTS);
    }

    for (auto& anim : leg_animators) {
        if (anim.target_pos.w == 0.0f)
            continue;

        anim.target_point_vao.update_vertex_data(
            reinterpret_cast<DebugShaderVertex*>(&anim.target_pos),
            1); // ugly, but it works

        anim.target_point_vao.draw(GL_POINTS);
    }

    if (render_data.draw_circles) {
        glUniform4f(render_data.debug_shader.color_loc, 0.3f, 0.6f, 1.0f, 1.0f);
        for (auto& anim : leg_animators) {
            anim.circle_vao.draw(GL_LINE_LOOP);
        }
    }
}