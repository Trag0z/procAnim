#pragma once
#include "pch.h"
#include "Color.h"
#include "Texture.h"
#include "Entity.h"
#include "Level.h"

class Renderer;
class LevelEditor;

struct BoxCollider {
    glm::vec2 position;
    glm::vec2 half_ext;

    // BoxCollider(glm::vec2 position, glm::vec2 half_ext);

    bool encloses_point(glm::vec2 point) const noexcept;

    float left_edge() const noexcept;
    float right_edge() const noexcept;
    float top_edge() const noexcept;
    float bottom_edge() const noexcept;

    glm::mat3 calculate_model_matrix() const noexcept;
};

struct CircleCollider {
    glm::vec2 position;
    float radius;
};

const BoxCollider*
find_first_collision_sweep_prune(CircleCollider circle, glm::vec2 velocity,
                                 std::list<BoxCollider> boxes);