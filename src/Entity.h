#pragma once
#include "pch.h"

class Entity {
  protected:
    glm::vec2 position_ = glm::vec2(1.0f);
    glm::vec2 scale = glm::vec2(1.0f);
    glm::mat3 model = glm::mat3(1.0f);

    const Entity* parent = nullptr;

    void update_model_matrix() {
        model = glm::translate(glm::mat3(1.0f), position_);
        model = glm::scale(model, scale);
    }
    void init(glm::vec2 pos_, glm::vec2 scale_,
              const Entity* parent_ = nullptr) {
        position_ = pos_;
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

    glm::vec2 position() const { return position_; }
};