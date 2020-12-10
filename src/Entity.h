#pragma once
#include "pch.h"

class Entity {
  protected:
    glm::vec2 position_ = glm::vec2(1.0f);
    glm::vec2 scale = glm::vec2(1.0f);
    glm::mat3 model = glm::mat3(1.0f);

    const Entity* parent = nullptr;

    void update_model_matrix();
    void init(glm::vec2 pos_ = glm::vec2(0.0f),
              glm::vec2 scale_ = glm::vec2(1.0f),
              const Entity* parent_ = nullptr);

  public:
    glm::vec2 world_to_local_space(const glm::vec2& world_pos) const;
    glm::vec2 local_to_world_space(const glm::vec2& local_pos) const;

    glm::vec2 world_to_world_scale(const glm::vec2& world_vec) const;
    glm::vec2 local_to_world_scale(const glm::vec2& local_vec) const;

    const glm::mat3& model_matrix() const;

    glm::vec2 position() const;
};