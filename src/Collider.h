#pragma once
#include "Entity.h"

class Entity;

typedef glm::vec2 Point;
typedef glm::vec2 Vector;

struct Segment {
    Point a, b;

    Vector line() const noexcept;
};

struct AABB {
    Point center;
    Vector half_ext;

    float min(int axis) const noexcept;
    float max(int axis) const noexcept;

    glm::mat3 calculate_model_matrix() const noexcept;
};

bool operator==(const AABB& lhs, const AABB& rhs) {
    return lhs.center == rhs.center && lhs.half_ext == rhs.half_ext;
}

struct Circle {
    Point center;
    float radius;

    Circle local_to_world_space(const Entity& entity) const noexcept;
};
