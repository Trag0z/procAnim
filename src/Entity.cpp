#pragma once

#include "Entity.h"

void Entity::update_model_matrix() {
    model = glm::translate(glm::mat3(1.0f), position_);
    model = glm::scale(model, scale);
}

void Entity::init(glm::vec2 pos_, glm::vec2 scale_) {
    position_ = pos_;
    scale = scale_;
    update_model_matrix();
}

glm::vec2 Entity::world_to_local_space(const glm::vec2& world_pos) const {
    return glm::inverse(model) * glm::vec3(world_pos, 1.0f);
}
glm::vec2 Entity::local_to_world_space(const glm::vec2& local_pos) const {
    return model * glm::vec3(local_pos, 1.0f);
}

glm::vec2 Entity::world_to_world_scale(const glm::vec2& world_vec) const {
    return {world_vec.x / scale.x, world_vec.y / scale.y};
}
glm::vec2 Entity::local_to_world_scale(const glm::vec2& local_vec) const {
    return {local_vec.x * scale.x, local_vec.y * scale.y};
}

float Entity::world_to_world_scale(const float world) const {
    SDL_assert(std::abs(scale.x) == std::abs(scale.y));
    return world / scale.x;
}
float Entity::local_to_world_scale(const float local) const {
    SDL_assert(std::abs(scale.x) == std::abs(scale.y));
    return local * scale.x;
}

const glm::mat3& Entity::model_matrix() const { return model; }

glm::vec2 Entity::position() const { return position_; }