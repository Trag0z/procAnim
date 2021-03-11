#pragma once
#include <vector>
#include "glm/glm.hpp"
#include "sdl/SDL_assert.h"
#include "CollisionDetection.h"
#include "Types.h"
#include "Util.h"

#define VERIFY_COLLISION_OUTCOMES

struct Ray {
    Point origin;
    Vector direction;
};

float squared_distance_point_AABB(Point p, AABB box) {
    float result = 0.0f;
    for (int i = 0; i < 2; ++i) {
        // For each axis count any axcess distance outside box extents
        float v = p[i];
        if (v < box.min(i)) {
            result += (box.min(i) - v) * (box.min(i) - v);
        } else if (v > box.max(i)) {
            result += (v - box.max(i)) * (v - box.max(i));
        }
    }
    return result;
}

// Returns the squared distance between line segment seg and point p (see
// Real-Time Collision Detection p.130)
float squared_distance_segment_point(const Segment& seg, const Point& p) {
    Point ab = seg.b - seg.a, ac = p - seg.a, bc = p - seg.b;
    float e = glm::dot(ac, ab);

    // Handle cases where c projects outside ab
    if (e <= 0.0f) return glm::dot(ac, ac);

    float f = glm::dot(ab, ab);
    if (e >= f) return glm::dot(bc, bc);

    // Handle cases where c projects onto ab
    return glm::dot(ac, ac) - e * e / f;
}

float signed_2D_tiangle_area(const Point& a, const Point& b, const Point& c) {
    return (a.x - c.x) * (b.y - c.y) * (b.x - c.x);
}

// Test if two segments overlap. If t and p are set, return intersection t value
// along seg1 and intersection point p
bool intersect_segment_segment(const Segment& seg1,
                               const Segment& seg2,
                               float* t,
                               Point* p) {
    // Sign of areas correspond to which side of ab points c and d are
    float a1 = signed_2D_tiangle_area(seg1.a, seg1.b, seg2.b);
    float a2 = signed_2D_tiangle_area(seg1.a, seg1.b, seg2.a);

    // If seg2.a and seg2.b are on different sides of seg1, areas have different
    // signs
    if (a1 * a2 < 0.0f) {
        // If this fires, the lines are collinear
        SDL_assert(a1 != 0.0f && a2 != 0.0f);

        float a3 = signed_2D_tiangle_area(seg2.a, seg2.b, seg1.a);
        // Since area is constant, a1 - a2 = a3 - a4 => a4 = a3 + a2 - a1
        float a4 = a3 + a2 - a1;
        // If seg1.a and seg1.b are on different sides of seg2, areas have
        // different signes
        if (a3 * a4 < 0.0f) {
            if (t) { *t = a3 / (a3 - a4); }
            if (p) {
                SDL_assert(t);
                *p = seg1.a + *t * (seg1.b - seg1.a);
            }
            return true;
        }
    }
    return false;
}

bool intersect_ray_circle(const Ray& ray,
                          const Circle& circle,
                          float* t = nullptr,
                          Point* p = nullptr) {
    Vector m = ray.origin - circle.center;
    float b  = glm::dot(m, ray.direction);
    float c  = glm::dot(m, m) - circle.radius * circle.radius;

    // Exit if ray's origin is outside the circle (c > 0) and the ray's
    // direction is pointing away from the circle (b > 0)
    if (c > 0.0f && b > 0.0f) return false;

    float discr = b * b - c;
    // A negative discriminant corresponds to the segment missing the circle
    if (discr < 0.0f) return false;

    // Ray now found to intersect sphere, compute smallest t value of
    // intersection
    if (t) {
        *t = -b - glm::sqrt(discr);

        // If t is negative, ray started inside circle so clamp t to zero
        if (*t < 0.0f) *t = 0.0f;
    }
    if (p) {
        SDL_assert(t);
        *p = *p + *t * ray.direction;
    }
    return true;
}

// Test if seg intersects the circle. Returns t value iof intersection and
// intersection point p
bool intersect_segment_circle(const Segment& seg,
                              const Circle& circle,
                              float* t,
                              Point* p,
                              Direction* dir) {
    Vector seg_vec   = seg.b - seg.a;
    float seg_length = glm::length(seg_vec);
    Ray ray          = { seg.a, seg_vec / seg_length };

    float t_if_nullptr;
    if (!t) { t = &t_if_nullptr; }
    Point p_if_nullptr;
    if (!p) { p = &p_if_nullptr; }

    if (intersect_ray_circle(ray, circle, t, p) && *t <= seg_length) {
        *t /= seg_length;

        if (dir) {
            Vector circle_to_p = *p - circle.center;
            float angle        = glm::atan(circle_to_p.y, circle_to_p.x);

            SDL_assert(circle_to_p != Vector(0.0f));

            constexpr float PI_4 = PI / 4.0f;
            if (angle > PI_4 && angle < PI_4 * 3.0f) {
                // Vector from circle center to intersection point (p) points
                // up, so (from the segments point of view) the intersection
                // direction is downwards. Same procedure for the other
                // directions.
                *dir = Direction::DOWN;
            } else if (angle < PI_4 && angle > -PI_4) {
                *dir = Direction::LEFT;
            } else if (angle < -PI_4 && angle > -PI_4 * 3.0f) {
                *dir = Direction::UP;
            } else {
                SDL_assert(angle > PI_4 * 3.0f || angle < -PI_4 * 3.0f);
                *dir = Direction::RIGHT;
            }
        }
        return true;
    }
    return false;
}

// Intersects ray R(t) = p + t * d against AABB box. When intersecting, return
// intersection distance t and point p of intersection (see
// Real-Time Collision Detection p.180)
bool intersect_ray_AABB(
  Ray ray, const AABB& box, float& t, Point& p, Direction& dir) {
    t           = 0.0f;
    float t_max = FLT_MAX;

    // For all two dimensions of the AABB
    for (int i = 0; i < 2; ++i) {
        if (std::abs(ray.direction[i]) < FLT_EPSILON) {
            // Ray is parallel to this side. No hit if origin is not within
            // [min, max] on this axis
            if (ray.origin[i] <= box.min(i) || ray.origin[i] >= box.max(i))
                return false;
        } else {
            // Compute intersection t value of ray with near and far side of
            // AABB
            float ood = 1.0f / ray.direction[i];
            float t1  = (box.min(i) - ray.origin[i]) * ood;
            float t2  = (box.max(i) - ray.origin[i]) * ood;
            // Make t1 be the intersection with near side, t2 with far side
            if (t1 > t2) std::swap(t1, t2);
            // Compute the intersection of slab intersection intervals
            if (t1 > t) {
                t = t1;

                // Set the collision direction
                if (i == 0) {  // x-axis
                    if (ray.direction.x < 0.0f) {
                        dir = Direction::LEFT;
                    } else {
                        dir = Direction::RIGHT;
                    }
                } else {  // y-axis
                    SDL_assert(i == 1);
                    if (ray.direction.y < 0.0f) {
                        dir = Direction::DOWN;
                    } else {
                        dir = Direction::UP;
                    }
                }
            }
            if (t2 < t_max) t_max = t2;

            // Exit with no collision as soon as side intersection becomes empty
            if (t > t_max || t_max <= 0.0f) return false;
        }
    }
    // Ray intersects on both dimensions. Return point (p) and intersection t
    // value (t)
    p = ray.origin + ray.direction * t;
    return true;
}

bool test_point_AABB(const Point& p, const AABB& box) {
    Vector v = glm::abs(box.center - p);
    return v.x < box.half_ext.x && v.y < box.half_ext.y;
}

bool test_circle_AABB(const Circle& circle, const AABB& box) {
    float squared_distance = squared_distance_point_AABB(circle.center, box);
    return squared_distance < circle.radius * circle.radius;
}

bool test_AABB_AABB(const AABB& a, const AABB& b) {
    Vector a_to_b           = b.center - a.center;
    Vector combined_extents = a.half_ext + b.half_ext;

    return glm::abs(a_to_b.x) < combined_extents.x
        && glm::abs(a_to_b.y) < combined_extents.y;
}

enum class Corner {
    BOTTOM_LEFT  = 0,
    BOTTOM_RIGHT = 1,
    TOP_LEFT     = 2,
    TOP_RIGHT    = 3,
    NONE         = 4
};

static Point corner(const AABB& box, Corner c) {
    Point result;
    result.x = (((uint)c & 1) ? box.max(0) : box.min(0));
    result.y = (((uint)c & 2) ? box.max(1) : box.min(1));
    return result;
}

// Applies a small margin to p in the opposite direction of dir so it does not
// intersect with box anymore
static Point apply_margins(Point p, Direction dir, const AABB& box) {
    const float MARGIN = 2.0f;
    if (dir == Direction::LEFT && p.x <= box.max(0)) {
        p.x = box.max(0) + MARGIN;
    } else if (dir == Direction::RIGHT && p.x >= box.min(0)) {
        p.x = box.min(0) - MARGIN;
    } else if (dir == Direction::DOWN && p.y <= box.max(1)) {
        p.y = box.max(1) + MARGIN;
    } else if (dir == Direction::UP && p.y >= box.min(1)) {
        p.y = box.min(1) - MARGIN;
    }
    return p;
}

static bool intersect_moving_circle_AABB(const Circle& circle,
                                         const Vector& move,
                                         const AABB& box,
                                         float& t,
                                         Point& p,
                                         Direction& dir) {

    // Compute the AABB resulting from expanding the box by circle.radius
    AABB expanded_box = box;
    expanded_box.half_ext += Vector(circle.radius);

    // Intersect ray against expanded_box. Exit with no itnersection if ray
    // misses, else get intersection point and time t as result
    if (!intersect_ray_AABB(
          Ray { circle.center, move }, expanded_box, t, p, dir))
        return false;

    // If the intersection lies in the Voronoi region of a corner, set
    // intersection_corner to that one
    Corner intersection_corner = Corner::NONE;
    if (p.x < box.min(0)) {
        if (p.y < box.min(1)) {
            intersection_corner = Corner::BOTTOM_LEFT;
        } else if (p.y > box.max(1)) {
            intersection_corner = Corner::BOTTOM_RIGHT;
        }
    } else if (p.x > box.max(0)) {
        if (p.y < box.min(1)) {
            intersection_corner = Corner::TOP_LEFT;
        } else if (p.y > box.max(1)) {
            intersection_corner = Corner::TOP_RIGHT;
        }
    }

    // Define line segment [c, c+d] specified by the circle movement
    Segment seg = { circle.center, circle.center + move };

    if (intersection_corner != Corner::NONE) {
        // Intersection point lies in the Voronoi region of a corner. The line
        // segment has to intersect the circle at this corner to actually count
        // as a hit.
        if (intersect_segment_circle(
              seg,
              Circle { corner(box, intersection_corner), circle.radius },
              &t,
              &p,
              &dir)) {
            p = apply_margins(p, dir, expanded_box);

#ifdef VERIFY_COLLISION_OUTCOMES
            {
                Circle final_pos { p, circle.radius };
                SDL_assert(!test_circle_AABB(final_pos, box));
            }
#endif
            return true;
        }

        return false;
    }

    p = apply_margins(p, dir, expanded_box);

#ifdef VERIFY_COLLISION_OUTCOMES
    SDL_assert(t != 0.0f);
    SDL_assert(!test_point_AABB(circle.center, expanded_box));
    {
        Circle final_pos { p, circle.radius };
        SDL_assert(!test_circle_AABB(final_pos, box));
    }
#endif

    // p is in an edge region. p and t are already correct.
    return true;
}

const CollisionData find_first_collision_moving_circle(
  const Circle& circle, const Vector move, const std::list<AABB>& boxes) {
    AABB culling_box;
    {
        Vector half_move       = move * 0.5f;
        culling_box.center     = circle.center + half_move;
        culling_box.half_ext.x = std::abs(half_move.x) + circle.radius + 5.0f;
        culling_box.half_ext.y = std::abs(half_move.y) + circle.radius + 5.0f;
    }

    std::vector<const AABB*> candidates;
    for (const auto& box : boxes) {
        if (test_AABB_AABB(culling_box, box)) { candidates.push_back(&box); }
    }

    CollisionData result {
        circle.center + move, 1.0f, Direction::NONE, nullptr
    };
    for (const auto& box : candidates) {
        float t;
        Point p;
        Direction dir;
        if (intersect_moving_circle_AABB(circle, move, *box, t, p, dir)) {
            if (t < result.t) {
                result.position   = p;
                result.t          = t;
                result.direction  = dir;
                result.hit_object = box;
            }
        }
    }

#ifdef VERIFY_COLLISION_OUTCOMES
    {
        Circle final_pos { result.position, circle.radius };
        for (const auto& box : candidates) {
            SDL_assert(!test_circle_AABB(final_pos, *box));
        }
        for (const auto& box : boxes) {
            SDL_assert(!test_circle_AABB(final_pos, box));
        }
    }
#endif

    return result;
}

const BallisticMoveResult
get_ballistic_move_result(const Circle& coll,
                          const Vector velocity,
                          const float delta_time,
                          const std::list<AABB>& level,
                          float rebound,
                          const size_t max_collision_iterations) {

    CollisionData collision;
    float remaining_time         = delta_time;
    Circle circle                = coll;
    Vector updated_velocity      = velocity;
    Direction last_hit_direction = Direction::NONE;
    const AABB* last_hit_object  = nullptr;

    for (size_t i = 0; i < max_collision_iterations; ++i) {
        collision = find_first_collision_moving_circle(
          circle, updated_velocity * remaining_time, level);

        if (collision.direction == Direction::NONE) {

#ifdef VERIFY_COLLISION_OUTCOMES
            {
                Circle final_pos { collision.position, circle.radius };
                for (const auto& box : level) {
                    SDL_assert(!test_circle_AABB(final_pos, box));
                }
            }
#endif

            return { collision.position,
                     updated_velocity,
                     last_hit_direction,
                     last_hit_object };

        } else if (collision.direction == Direction::LEFT
                   || collision.direction == Direction::RIGHT) {
            updated_velocity.x *= -rebound;
        } else {
            SDL_assert(collision.direction == Direction::UP
                       || collision.direction == Direction::DOWN);
            updated_velocity.y *= -rebound;
        }

        remaining_time -= remaining_time * collision.t;
        circle.center      = collision.position;
        last_hit_direction = collision.direction;
        last_hit_object    = collision.hit_object;
    }
    SDL_TriggerBreakpoint();
    return {};
}