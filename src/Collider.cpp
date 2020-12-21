#pragma once
#include "pch.h"
#include "Collider.h"
#include "Renderer.h"

// BoxCollider::BoxCollider(glm::vec2 position, glm::vec2 half_ext) {
//     position = position;
//     half_ext = half_ext;
// }

bool BoxCollider::encloses_point(glm::vec2 point) const noexcept {
    return point.x > position.x - half_ext.x &&
           point.x < position.x + half_ext.x &&
           point.y > position.y - half_ext.y &&
           point.y < position.y + half_ext.y;
}

float BoxCollider::left_edge() const noexcept {
    SDL_assert(half_ext.x > 0.0f);
    return position.x - half_ext.x;
}

float BoxCollider::right_edge() const noexcept {
    SDL_assert(half_ext.x > 0.0f);
    return position.x + half_ext.x;
}

float BoxCollider::top_edge() const noexcept {
    SDL_assert(half_ext.y > 0.0f);
    return position.y + half_ext.y;
}

float BoxCollider::bottom_edge() const noexcept {
    SDL_assert(half_ext.y > 0.0f);
    return position.y - half_ext.y;
}

glm::mat3 BoxCollider::calculate_model_matrix() const noexcept {
    glm::mat3 model = glm::translate(glm::mat3(1.0f), position);
    return glm::scale(model, half_ext);
}

// const BoxCollider*
// find_first_collision_sweep_prune(CircleCollider circle, glm::vec2 velocity,
//                                  std::list<BoxCollider> boxes) {}