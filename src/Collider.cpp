#pragma once
#include "pch.h"
#include "Collider.h"
#include "Renderer.h"

BoxCollider::BoxCollider(glm::vec2 position, glm::vec2 half_ext) {
    Entity::init(position, half_ext);
    half_ext_ = half_ext;
}

void BoxCollider::update_model_matrix() {
    scale = half_ext_;
    Entity::update_model_matrix();
}

glm::vec2 BoxCollider::half_ext() const { return half_ext_; }

bool BoxCollider::encloses_point(glm::vec2 point) const noexcept {
    return point.x > position_.x - half_ext_.x &&
           point.x < position_.x + half_ext_.x &&
           point.y > position_.y - half_ext_.y &&
           point.y < position_.y + half_ext_.y;
}

float BoxCollider::left_edge() const noexcept {
    SDL_assert(half_ext_.x > 0.0f);
    return position_.x - half_ext_.x;
}

float BoxCollider::right_edge() const noexcept {
    SDL_assert(half_ext_.x > 0.0f);
    return position_.x + half_ext_.x;
}

float BoxCollider::top_edge() const noexcept {
    SDL_assert(half_ext_.y > 0.0f);
    return position_.y + half_ext_.y;
}

float BoxCollider::bottom_edge() const noexcept {
    SDL_assert(half_ext_.y > 0.0f);
    return position_.y - half_ext_.y;
}