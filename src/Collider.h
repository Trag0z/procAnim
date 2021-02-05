#pragma once
#include "pch.h"
#include "Entity.h"

struct LineCollider;
class Entity;

enum Direction { NONE, UP, DOWN, LEFT, RIGHT };

// NOTE: All intersects() functions should return false if the objects' edges
// exactly overlap

struct BoxCollider {
    glm::vec2 position;
    glm::vec2 half_ext;

    bool intersects(const BoxCollider& other) const noexcept;
    bool encloses_point(glm::vec2 point) const noexcept;

    float left_edge() const noexcept;
    float right_edge() const noexcept;
    float top_edge() const noexcept;
    float bottom_edge() const noexcept;

    LineCollider edge_collider(Direction dir) const noexcept;

    glm::mat3 calculate_model_matrix() const noexcept;
};

struct CircleCollider {
    glm::vec2 position;
    float radius;

    bool intersects(const BoxCollider& other) const noexcept;
    bool intersects(const CircleCollider& other) const noexcept;

    CircleCollider local_to_world_space(const Entity& entity) const noexcept;
};

struct LineCollider {
    glm::vec2 start;
    glm::vec2 line;

    LineCollider() {}
    LineCollider(glm::vec2 start, glm::vec2 line) : start(start), line(line) {}

    bool intersects(const BoxCollider& other, float* out_collision_time,
                    Direction* out_hit_side) const noexcept;
    bool intersects(const LineCollider& other,
                    float* out = nullptr) const noexcept;
    bool intersects(const CircleCollider& other) const noexcept;
};

struct CollisionData {
    glm::vec2 move_until_collision;
    float time;
    Direction direction;
};

const CollisionData
find_first_collision_sweep_prune(const CircleCollider& circle,
                                 const glm::vec2 move,
                                 const std::list<BoxCollider>& boxes);

struct BallisticMoveResult {
    glm::vec2 new_position;
    glm::vec2 new_velocity;
};

const BallisticMoveResult get_ballistic_move_result(
    const CircleCollider& coll, const glm::vec2 velocity,
    const float delta_time, const std::list<BoxCollider>& level,
    float rebound = 1.0f, const size_t max_collision_iterations = 5);