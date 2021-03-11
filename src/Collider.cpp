#pragma once

#include "Collider.h"
#include "Util.h"
#include "CollisionDetection.h"

Vector Segment::line() const noexcept {
    return b - a;
}

float AABB::min(int axis) const noexcept {
    SDL_assert(half_ext.x > 0.0f && half_ext.y > 0.0f);
    SDL_assert(axis < 2);
    return center[axis] - half_ext[axis];
}
float AABB::max(int axis) const noexcept {
    SDL_assert(half_ext.x > 0.0f && half_ext.y > 0.0f);
    SDL_assert(axis < 2);
    return center[axis] + half_ext[axis];
}

glm::mat3 AABB::calculate_model_matrix() const noexcept {
    glm::mat3 model = glm::translate(glm::mat3(1.0f), center);
    return glm::scale(model, half_ext);
}

Circle Circle::local_to_world_space(const Entity& entity) const noexcept {
    return { entity.local_to_world_space(center),
             entity.local_to_world_scale(radius) };
}
