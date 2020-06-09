#pragma once
#include "pch.h"

class Entity {
  protected:
    glm::vec3 pos = glm::vec3(1.0f);
    glm::vec3 scale = glm::vec3(1.0f);
    glm::mat4 model = glm::mat4(1.0f);

    const Entity* parent = nullptr;

    void update_model_matrix() {
        model = glm::translate(glm::mat4(1.0f), pos);
        model = glm::scale(model, scale);
    }
    void init(glm::vec3 pos_, glm::vec3 scale_,
              const Entity* parent_ = nullptr) {
        pos = pos_;
        scale = scale_;
        update_model_matrix();
        parent = parent_;
    };

  public:
    glm::vec3 world_to_local_space(const glm::vec3& world_pos) const {
        return glm::inverse(model) * glm::vec4(world_pos, 1.0f);
    };

    glm::vec4 world_to_local_space(const glm::vec4& world_pos) const {
        return glm::inverse(model) * world_pos;
    }

    glm::vec3 local_to_world_space(const glm::vec3& local_pos) const {
        return model * glm::vec4(local_pos, 1.0f);
    };

    glm::vec4 local_to_world_space(const glm::vec4& local_pos) const {
        return model * local_pos;
    };

    //     void set_pos(glm::vec3 p) {
    //         pos = p;
    //         update_model_matrix();
    //     }
    //     const glm::vec3& get_pos() const { return pos; }

    //     void set_scale(glm::vec3 s) {
    //         scale = s;
    //         update_model_matrix();
    //     }
    //     const glm::vec3& get_scale() const { return scale; }

    //     const glm::mat4& get_model() const { return model; }
};