#pragma once
#include "pch.h"

class Entity {
  protected:
    glm::vec2 position = glm::vec2(1.0f);
    glm::vec2 scale = glm::vec2(1.0f);
    glm::mat3 model = glm::mat3(1.0f);

    const Entity* parent = nullptr;

    void update_model_matrix() {
        model = glm::translate(glm::mat3(1.0f), position);
        model = glm::scale(model, scale);
    }
    void init(glm::vec2 pos_ = glm::vec2(0.0f),
              glm::vec2 scale_ = glm::vec2(1.0f),
              const Entity* parent_ = nullptr) {
        position = pos_;
        scale = scale_;
        update_model_matrix();
        parent = parent_;
    };

  public:
    glm::vec2 world_to_local_space(const glm::vec2& world_pos) const {
        return glm::inverse(model) * glm::vec3(world_pos, 1.0f);
    }
    glm::vec2 local_to_world_space(const glm::vec2& local_pos) const {
        return model * glm::vec3(local_pos, 1.0f);
    }
    glm::vec2 world_to_world_scale(const glm::vec2& world_vec) const {
        return {world_vec.x / scale.x, world_vec.y / scale.y};
    }
    glm::vec2 local_to_world_scale(const glm::vec2& local_vec) const {
        return {local_vec.x * scale.x, local_vec.y * scale.y};
    }

    // // TODO: These have to change, can't just scale a float by a vector
    // float world_to_local_space(float world_length) const {
    //     SDL_assert(std::abs(scale.x) == std::abs(scale.y));
    //     return world_length / std::abs(scale.x);
    // }
    // float local_to_world_space(float local_length) const {
    //     SDL_assert(std::abs(scale.x) == std::abs(scale.y));
    //     return std::abs(scale.x) * local_length;
    // }

    const glm::mat3& get_model_matrix() const { return model; }

    glm::vec2 get_position() const { return position; }
};