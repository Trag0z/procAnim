#pragma once

#include "CollisionDetection.h"
#include "Types.h"

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
    if (e <= 0.0f)
        return glm::dot(ac, ac);

    float f = glm::dot(ab, ab);
    if (e >= f)
        return glm::dot(bc, bc);

    // Handle cases where c projects onto ab
    return glm::dot(ac, ac) - e * e / f;
}

float signed_2D_tiangle_area(const Point& a, const Point& b, const Point& c) {
    return (a.x - c.x) * (b.y - c.y) * (b.x - c.x);
}

// Test if two segments overlap. If t and p are set, return intersection t value
// along seg1 and intersection point p
bool intersect_segment_segment(const Segment& seg1, const Segment& seg2,
                               float* t, Point* p) {
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
            if (t) {
                *t = a3 / (a3 - a4);
            }
            if (p) {
                SDL_assert(t);
                *p = seg1.a + *t * (seg1.b - seg1.a);
            }
            return true;
        }
    }
    return false;
}

bool intersect_ray_circle(const Ray& ray, const Circle& circle,
                          float* t = nullptr, Point* p = nullptr) {
    Point m = ray.origin - circle.center;
    float b = glm::dot(m, ray.direction);
    float c = glm::dot(m, m) - circle.radius * circle.radius;
    // Exit if ray's origin is outside the circle (c > 0) and the ray's
    // direction is pointing away from the circle (b > 0)
    if (c > 0.0f && b > 0.0f)
        return false;
    float discr = b * b - c;
    // A negative discriminant corresponds to the segment missing the circle
    if (discr < 0.0f)
        return false;
    // Ray now found to intersect sphere, compute smallest t value of
    // intersection
    if (t) {
        *t = -b - glm::sqrt(discr);
        // If t is negative, ray started inside circle so clamp t to zero
        if (*t < 0.0f)
            *t = 0.0f;
    }
    if (p) {
        SDL_assert(t);
        *p = *p + *t * ray.direction;
    }
    return true;
}

// Test if seg intersects the circle. Returns t value iof intersection and
// intersection point p
bool intersect_segment_circle(const Segment& seg, const Circle& circle,
                              float* t, Point* p) {
    Vector seg_vec = seg.b - seg.a;
    float seg_length = glm::length(seg_vec);
    Ray ray = {seg.a, seg_vec / seg_length};

    float t_if_nullptr;
    if (!t) {
        t = &t_if_nullptr;
    }

    if (intersect_ray_circle(ray, circle, t, p) && *t <= seg_length)
        return true;
    return false;
}

// Intersects ray R(t) = p + t * d against AABB a. When intersecting, return
// intersection distance t and point q of intersection (see
// Real-Time Collision Detection p.180)
bool intersect_ray_AABB(const Point& p, const Point& d, const AABB& a, float& t,
                        Point& q) {
    t = 0.0f;
    float t_max = FLT_MAX;

    // For all two dimensions of the AABB
    for (int i = 0; i < 2; ++i) {
        if (std::abs(d[i]) < FLT_EPSILON) {
            // Ray is parallel to this side. No hit if origin is not within
            // [min, max] on this axis
            if (p[i] < a.min(i) || p[i] > a.max(i))
                return false;
        } else {
            // Compute intersection t value of ray with near and far side of
            // AABB
            float ood = 1.0f / d[i];
            float t1 = (a.min(i) - p[i]) * ood;
            float t2 = (a.max(i) - p[i]) * ood;
            // Make t1 be the intersection with near side, t2 with far side
            if (t1 > t2)
                std::swap(t1, t2);
            // Compute the intersection of slab intersection intervals
            if (t1 > t)
                t = t1;
            if (t2 > t_max)
                t_max = t2;
            // Exit with no collision as soon as side intersection becomes empty
            if (t > t_max)
                return false;
        }
    }
    // Ray intersects on both dimensions. Return point (q) and intersection t
    // value (t)
    q = p + d * t;
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
    Vector a_to_b = b.center - a.center;
    Vector combined_extents = a.half_ext + b.half_ext;

    return glm::abs(a_to_b.x) < combined_extents.x &&
           glm::abs(a_to_b.y) < combined_extents.y;
}

enum class Corner {
    BOTTOM_LEFT = 0,
    BOTTOM_RIGHT = 1,
    TOP_LEFT = 2,
    TOP_RIGHT = 3,
    NONE = 4
};

static Point corner(const AABB& box, Corner c) {
    Point result;
    result.x = (((uint)c & 1) ? box.max(0) : box.min(0));
    result.y = (((uint)c & 2) ? box.max(1) : box.min(1));
    return result;
}

static bool intersect_moving_circle_AABB(const Circle& circle,
                                         const Vector& move, const AABB& box,
                                         float& t, Point& p) {
    // Compute the AABB resulting from expanding the box by circle.radius
    AABB expanded_box = box;
    expanded_box.half_ext += glm::vec2(circle.radius);

    // Intersect ray against expanded_box. Exit with no itnersection if ray
    // misses, else get intersection point and time t as result
    if (!intersect_ray_AABB(circle.center, move, box, t, p))
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
    Segment seg = {circle.center, circle.center + move};

    if (intersection_corner != Corner::NONE) {
        // Intersection point lies in the Voronoi region of a corner. The line
        // segment has to intersect the circle at this corner to actually count
        // as a hit.
        if (intersect_segment_circle(
                seg, Circle{corner(box, intersection_corner), circle.radius},
                &t, &p))
            return true;

        return false;
    }

    // p is in an edge region. p and t are already correct.
    return true;
}

const CollisionData
find_first_collision_moving_circle(const Circle& circle, const Vector move,
                                   const std::list<AABB>& boxes) {
    AABB culling_box;
    {
        Vector half_move = move * 0.5f;
        culling_box.center = circle.center + half_move;
        culling_box.half_ext.x = std::abs(half_move.x) + circle.radius + 5.0f;
        culling_box.half_ext.y = std::abs(half_move.y) + circle.radius + 5.0f;
    }

    std::vector<const AABB*> candidates;
    for (const auto& box : boxes) {
        if (test_AABB_AABB(culling_box, box)) {
            candidates.push_back(&box);
        }
    }

    CollisionData result{1.0f, Direction::NONE, nullptr};
    for (const auto& box : candidates) {
        float t;
        Point p;
        if (intersect_moving_circle_AABB(circle, move, *box, t, p)) {
            if (t < result.t) {
                result.t = t;
                result.hit_object = box;

                // Compute the vector from the circle's center at collision time
                // to the intersection point (p). It points in the direction
                // where the collision happened. Map the direction to one of the
                // 4 directions for the return value.
                Point center_at_collision_time = circle.center + move * t;
                Vector c_p = p - center_at_collision_time;
                if (glm::abs(c_p.x) > glm::abs(c_p.y)) {
                    result.direction =
                        ((c_p.x < 0.0f) ? Direction::LEFT : Direction::RIGHT);
                } else {
                    result.direction =
                        ((c_p.y < 0.0f) ? Direction::DOWN : Direction::UP);
                }
            }
        }
    }

    return result;
}

// const CollisionData find_first_collision(const CircleCollider& circle,
//                                          const glm::vec2 move,
//                                          const std::list<BoxCollider>& level)
//                                          {
//     const CircleCollider new_circle = {circle.center + move, circle.radius};
//     CollisionData result = {move, 1.0f, NONE, nullptr};

// #if _DEBUG
//     for (const auto& box : level) {
//         SDL_assert(!circle.intersects(box));
//     }
// #endif

//     if (move == glm::vec2(0.0f)) {
//         return result;
//     }

//     for (const auto& box : level) {
//         if (new_circle.intersects(box)) {
//             glm::vec2 center_diff = new_circle.center - box.center;
//             glm::vec2 distance_inside_box =
//                 center_diff - box.half_ext - glm::vec2(new_circle.radius);

//             glm::vec2 time_to_collision = {distance_inside_box.x / move.x,
//                                            distance_inside_box.y / move.y};

//             float first_collision_time = 1.0f;
//             Direction collision_direction = NONE;

//             if (isfinite(time_to_collision.x) &&
//                 isfinite(time_to_collision.y)) {
//                 if (time_to_collision.x < time_to_collision.y) {
//                     first_collision_time = time_to_collision.x;

//                     if (move.x < 0.0f) {
//                         collision_direction = LEFT;
//                     } else {
//                         collision_direction = RIGHT;
//                     }
//                 } else {
//                     first_collision_time = time_to_collision.y;

//                     if (move.y < 0.0f) {
//                         collision_direction = DOWN;
//                     } else {
//                         collision_direction = UP;
//                     }
//                 }
//             } else if (isfinite(time_to_collision.x)) {
//                 first_collision_time = time_to_collision.x;

//                 if (move.x < 0.0f) {
//                     collision_direction = LEFT;
//                 } else {
//                     collision_direction = RIGHT;
//                 }
//             } else if (isfinite(time_to_collision.y)) {
//                 first_collision_time = time_to_collision.y;

//                 if (move.y < 0.0f) {
//                     collision_direction = DOWN;
//                 } else {
//                     collision_direction = UP;
//                 }
//             } else {
//                 SDL_TriggerBreakpoint();
//             }

//             SDL_assert(first_collision_time > 0.0f);
//             if (first_collision_time > 0.0f &&
//                 first_collision_time < result.t) {
//                 result = {
//                     move * first_collision_time,
//                     first_collision_time,
//                     collision_direction,
//                     &box,
//                 };
//             }
//         }
//     }

//     if (result.direction == UP) {
//         result.move.y = (circle.center.y + circle.radius) -
//                         result.hit_object->bottom_edge();

//     } else if (result.direction == DOWN) {
//         result.move.y =
//             circle.center.y - circle.radius - result.hit_object->top_edge();

//     } else if (result.direction == LEFT) {
//         result.move.x =
//             (circle.center.x - circle.radius) -
//             result.hit_object->right_edge();

//     } else if (result.direction == RIGHT) {
//         result.move.x =
//             (circle.center.x + circle.radius) -
//             result.hit_object->left_edge();
//     }

// #if _DEBUG
//     const CircleCollider result_circle = {circle.center + result.move,
//                                           circle.radius};
//     if (result.direction != NONE) {
//         SDL_assert(!result_circle.intersects(*result.hit_object));
//     }
//     for (const auto& box : level) {
//         SDL_assert(!result_circle.intersects(box));
//     }
// #endif

//     return result;
// }

// const CollisionData
// find_first_collision_sweep_prune(const CircleCollider& circle,
//                                  const glm::vec2 move,
//                                  const std::list<BoxCollider>& boxes) {

//     // All colliders that are outside the culling box won't collide, so we
//     // don't have to care about them
//     BoxCollider culling_box;
//     {
//         glm::vec2 half_move = move * 0.5f;
//         culling_box.center = circle.center + half_move;
//         culling_box.half_ext.x = std::abs(half_move.x) + circle.radius
//         + 5.0f; culling_box.half_ext.y = std::abs(half_move.y) +
//         circle.radius + 5.0f;
//     }

//     std::vector<const BoxCollider*> candidates;
//     for (const auto& box : boxes) {
//         if (culling_box.intersects(box)) {
//             candidates.push_back(&box);
//         }
//     }

//     if (move == glm::vec2(0.0f)) {

//         // @CLEANUP: Commenting this out does not seem to result in bugs,
//         // but it probably should not fire anyway for the system to be
//         // robust.
//         //#ifdef _DEBUG
//         //        auto coll_iter = std::find_if(
//         //            candidates.begin(), candidates.end(),
//         //            [&circle](const auto& b) { return
//         //            circle.intersects(*b);
//         //            });
//         //
//         //        if (coll_iter != candidates.end()) {
//         //            SDL_TriggerBreakpoint();
//         //            circle.intersects(**coll_iter);
//         //        }
//         //#endif // DEBUG

//         return {glm::vec2(0.0f), 1.0f, Direction::NONE, nullptr};
//     }

//     glm::vec2 normals[2] = {glm::vec2(-move.y, move.x),
//                             glm::vec2(move.y, -move.x)};

//     LineCollider sweep_collider_lines[2] = {
//         {circle.center + glm::normalize(normals[0]) * circle.radius, move},
//         {circle.center + glm::normalize(normals[1]) * circle.radius, move},
//     };

//     float first_collision_time = 1.0f;
//     Direction first_collision_direction = NONE;
//     const BoxCollider* first_collision_object = nullptr;

//     // Find collisions with the collider lines
//     for (const auto cand : candidates) {
//         float collision_time = 1.0f;
//         Direction hit_side;

//         for (const auto& line : sweep_collider_lines) {
//             if (line.intersects(*cand, &collision_time, &hit_side) &&
//                 collision_time < first_collision_time) {
//                 SDL_assert(collision_time >= 0.0f);

//                 first_collision_time = collision_time;
//                 first_collision_object = cand;

//                 if (hit_side == UP) {
//                     first_collision_direction = DOWN;
//                 } else if (hit_side == DOWN) {
//                     first_collision_direction = UP;
//                 } else if (hit_side == LEFT) {
//                     first_collision_direction = RIGHT;
//                 } else if (hit_side == RIGHT) {
//                     first_collision_direction = LEFT;
//                 } else {
//                     SDL_TriggerBreakpoint();
//                 }
//             }
//         }
//     }

//     // Some part of the circle might still collide with something at the end
//     // point, so let's check for that
//     const CircleCollider circle_after_move = {
//         circle.center + move * first_collision_time, circle.radius};

//     for (const auto cand : candidates) {
//         if (circle_after_move.intersects(*cand)) {
//             glm::vec2 overlap;

//             overlap.x = (circle_after_move.radius + cand->half_ext.x) -
//                         std::abs(circle_after_move.center.x -
//                         cand->center.x);
//             SDL_assert(overlap.x > 0.0f);

//             overlap.y = (circle_after_move.radius + cand->half_ext.y) -
//                         std::abs(circle_after_move.center.y -
//                         cand->center.y);
//             SDL_assert(overlap.y > 0.0f);
//             // SDL_assert(overlap.x <= std::abs(move.x) || overlap.y <=
//             // std::abs(move.y));

//             SDL_assert(move != glm::vec2(0.0f));

//             glm::vec2 collision_time = glm::vec2(-0.1f);
//             if (move.x != 0.0f) {
//                 collision_time.x = 1.0f - overlap.x / std::abs(move.x);
//             }
//             if (move.y != 0.0f) {
//                 collision_time.y = 1.0f - overlap.y / std::abs(move.y);
//             }

//             SDL_assert(collision_time.x <= 1.0f && collision_time.y <= 1.0f);
//             if (collision_time.x <= -0.1f && collision_time.y <= -0.1f) {
//                 continue;
//             }

//             // Choose the later time because that's when the circle starts
//             // touching the candidate box, so it's the furthest it can move
//             if (collision_time.x > collision_time.y) {
//                 if (collision_time.x < first_collision_time) {

//                     first_collision_time = collision_time.x;
//                     first_collision_object = cand;

//                     if (move.x > 0.0f) {
//                         first_collision_direction = RIGHT;
//                     } else {
//                         first_collision_direction = LEFT;
//                         SDL_assert(move.x != 0.0f);
//                     }
//                 }
//             } else { // colision_times.y > collision_time.x
//                 if (collision_time.y < first_collision_time) {

//                     first_collision_time = collision_time.y;
//                     first_collision_object = cand;

//                     if (move.y > 0.0f) {
//                         first_collision_direction = UP;
//                     } else {
//                         first_collision_direction = DOWN;
//                         SDL_assert(move.y != 0.0f);
//                     }
//                 }
//             }
//         }
//     }

//     CollisionData result;
//     result.move = move * first_collision_time;
//     result.t = first_collision_time;
//     result.direction = first_collision_direction;
//     result.hit_object = first_collision_object;

//     // NOTE: These margins seem to be pretty large, but without them we hit
//     // assertions (due to floating point imprecision?)
//     if (first_collision_direction == UP) {
//         result.move.y -= 2.0f;
//     } else if (first_collision_direction == DOWN) {
//         result.move.y += 2.0f;
//     } else if (first_collision_direction == LEFT) {
//         result.move.x += 2.0f;
//     } else if (first_collision_direction == RIGHT) {
//         result.move.x -= 2.0f;
//     }

// #ifdef _DEBUG
//     CircleCollider after_move = {circle.center + result.move, circle.radius};
//     for (const auto& coll : candidates) {
//         if (after_move.intersects(*coll)) {
//             // SDL_TriggerBreakpoint();
//             after_move.intersects(*coll);
//         }
//     }
// #endif

//     return result;
// }

const BallisticMoveResult
get_ballistic_move_result(const Circle& coll, const Vector velocity,
                          const float delta_time, const std::list<AABB>& level,
                          float rebound,
                          const size_t max_collision_iterations) {

    // Vector move = velocity * delta_time;
    // CollisionData collision;

    // collision = find_first_collision_moving_circle(coll, move, level);

    // // NOTE: This condition could be removed and we would still have the
    // // same result, but it seems more clear (and efficient) to have it.
    // if (collision.direction == Direction::NONE) {
    //     return {coll.center + move, velocity, collision.direction,
    //             collision.hit_object};
    // }

    // Vector updated_velocity = velocity;
    // Circle updated_collider = coll;
    // Direction last_hit_direction = collision.direction;
    // const AABB* last_hit_object = collision.hit_object;

    // float remaining_time = delta_time;

    // for (size_t num_iterations = 1; num_iterations <
    // max_collision_iterations;
    //      ++num_iterations) {

    //     remaining_time -= remaining_time * collision.t;
    //     updated_collider.center += ;

    //     if (collision.direction == Direction::NONE) {
    //         return {updated_collider.center, updated_velocity,
    //                 last_hit_direction, last_hit_object};

    //     } else if (collision.direction == Direction::LEFT ||
    //                collision.direction == Direction::RIGHT) {
    //         updated_velocity.x *= -rebound;
    //     } else {
    //         SDL_assert(collision.direction == Direction::UP ||
    //                    collision.direction == Direction::DOWN);
    //         updated_velocity.y *= -rebound;
    //     }
    //     last_hit_direction = collision.direction;
    //     last_hit_object = collision.hit_object;

    //     collision = find_first_collision_moving_circle(
    //         updated_collider, updated_velocity * remaining_time, level);
    // }
    // SDL_TriggerBreakpoint();
    // return {};

    CollisionData collision;
    float remaining_time = delta_time;
    Circle circle = coll;
    Vector updated_velocity = velocity;
    Direction last_hit_direction = Direction::NONE;
    const AABB* last_hit_object = nullptr;

    for (size_t i = 0; i < max_collision_iterations; ++i) {
        collision = find_first_collision_moving_circle(
            circle, updated_velocity * remaining_time, level);

        circle.center += updated_velocity * collision.t;

        if (collision.direction == Direction::NONE) {
            return {circle.center, updated_velocity, last_hit_direction,
                    last_hit_object};

        } else if (collision.direction == Direction::LEFT ||
                   collision.direction == Direction::RIGHT) {
            updated_velocity.x *= -rebound;
        } else {
            SDL_assert(collision.direction == Direction::UP ||
                       collision.direction == Direction::DOWN);
            updated_velocity.y *= -rebound;
        }

        remaining_time -= remaining_time * collision.t;
        last_hit_direction = collision.direction;
        last_hit_object = collision.hit_object;
    }
    SDL_TriggerBreakpoint();
    return {};
}