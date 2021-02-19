#pragma once
#include "MemArena.h"
#include "Collider.h"

enum class Direction { NONE, UP, DOWN, LEFT, RIGHT };

bool test_point_AABB(const Point& p, const AABB& box);

bool test_circle_AABB(const Circle& circle, const AABB& box);

bool intersect_segment_segment(const Segment& seg1, const Segment& seg2,
                               float* t = nullptr, Point* p = nullptr);

bool intersect_segment_circle(const Segment& seg, const Circle& circle,
                              float* t = nullptr, Point* p = nullptr,
                              Direction* dir = nullptr);

struct CollisionData {
    Point position;
    float t;
    Direction direction;
    const AABB* hit_object;
};

const CollisionData
find_first_collision_moving_circle(const Circle& circle, const Vector move,
                                   const MemArena<AABB>& level);

struct BallisticMoveResult {
    glm::vec2 new_position;
    glm::vec2 new_velocity;
    Direction last_hit_diretcion;
    const AABB* last_hit_object;
};

const BallisticMoveResult
get_ballistic_move_result(const Circle& coll, const Vector velocity,
                          const float delta_time, const MemArena<AABB>& level,
                          float rebound = 1.0f,
                          const size_t max_collision_iterations = 5);
