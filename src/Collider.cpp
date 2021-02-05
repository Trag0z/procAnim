#pragma once
#include "pch.h"
#include "Collider.h"
#include "Util.h"

bool BoxCollider::intersects(const BoxCollider& other) const noexcept {
    SDL_assert(half_ext.x >= 0.0f && half_ext.y >= 0.0f);
    SDL_assert(other.half_ext.x >= 0.0f && other.half_ext.y >= 0.0f);

    return (std::abs(position.x - other.position.x) <
            half_ext.x + other.half_ext.x) &&
           (std::abs(position.y - other.position.y) <
            half_ext.y + other.half_ext.y);
}

bool BoxCollider::encloses_point(glm::vec2 point) const noexcept {
    return point.x > position.x - half_ext.x &&
           point.x < position.x + half_ext.x &&
           point.y > position.y - half_ext.y &&
           point.y < position.y + half_ext.y;
}

float BoxCollider::left_edge() const noexcept {
    SDL_assert(half_ext.x > 0.0f);
    return position.x - half_ext.x;
}

float BoxCollider::right_edge() const noexcept {
    SDL_assert(half_ext.x > 0.0f);
    return position.x + half_ext.x;
}

float BoxCollider::top_edge() const noexcept {
    SDL_assert(half_ext.y > 0.0f);
    return position.y + half_ext.y;
}

float BoxCollider::bottom_edge() const noexcept {
    SDL_assert(half_ext.y > 0.0f);
    return position.y - half_ext.y;
}

LineCollider BoxCollider::edge_collider(Direction dir) const noexcept {
    SDL_assert(half_ext.x >= 0.0f && half_ext.y >= 0.0f);
    SDL_assert(dir != NONE);

    if (dir == UP) {
        return {position + half_ext, glm::vec2(-half_ext.x * 2.0f, 0.0f)};
    } else if (dir == DOWN) {
        return {position - half_ext, glm::vec2(half_ext.x * 2.0f, 0.0f)};
    } else if (dir == LEFT) {
        return {position - half_ext, glm::vec2(0.0f, half_ext.y * 2.0f)};
    } else { // dir == RIGHT
        return {position + half_ext, glm::vec2(0.0f, -half_ext.y * 2.0f)};
    }
}

glm::mat3 BoxCollider::calculate_model_matrix() const noexcept {
    glm::mat3 model = glm::translate(glm::mat3(1.0f), position);
    return glm::scale(model, half_ext);
}

bool CircleCollider::intersects(const BoxCollider& other) const noexcept {
    SDL_assert(radius >= 0.0f);
    SDL_assert(other.half_ext.x >= 0.0f && other.half_ext.y >= 0.0f);

    glm::vec2 closest_point_in_rect =
        glm::vec2(clamp(position.x, other.left_edge(), other.right_edge()),
                  clamp(position.y, other.bottom_edge(), other.top_edge()));

    float distance_squared = length_squared(position - closest_point_in_rect);
    return distance_squared < (radius * radius);
}

bool CircleCollider::intersects(const CircleCollider& other) const noexcept {
    SDL_assert(radius >= 0.0f);
    SDL_assert(other.radius >= 0.0f);

    return glm::length(position - other.position) < radius + other.radius;
}

CircleCollider
CircleCollider::local_to_world_space(const Entity& entity) const noexcept {
    return {entity.local_to_world_space(position),
            entity.local_to_world_scale(radius)};
}

bool LineCollider::intersects(
    const BoxCollider& other, float* out_collision_time = nullptr,
    Direction* out_hit_side = nullptr) const noexcept {

    float first_collision_time = 1.0f;
    Direction first_hit_side = NONE;

    float collision_time = 1.1f;

    if (intersects(other.edge_collider(UP), &collision_time) &&
        collision_time <= first_collision_time) {
        first_collision_time = collision_time;
        first_hit_side = UP;
    }
    if (intersects(other.edge_collider(DOWN), &collision_time) &&
        collision_time <= first_collision_time) {
        first_collision_time = collision_time;
        first_hit_side = DOWN;
    }
    if (intersects(other.edge_collider(LEFT), &collision_time) &&
        collision_time <= first_collision_time) {
        first_collision_time = collision_time;
        first_hit_side = LEFT;
    }
    if (intersects(other.edge_collider(RIGHT), &collision_time) &&
        collision_time <= first_collision_time) {
        first_collision_time = collision_time;
        first_hit_side = RIGHT;
    }

    if (out_collision_time)
        *out_collision_time = first_collision_time;
    if (out_hit_side)
        *out_hit_side = first_hit_side;

    return first_hit_side != NONE;
}

bool LineCollider::intersects(const LineCollider& other,
                              float* out_t) const noexcept {
    float length1 = glm::length(line);
    float length2 = glm::length(other.line);

    // If the line start points are further apart than the
    // sum of the lenghts, they can't collide.
    if (glm::length(start - other.start) > length1 + length2) {
        return false;
    }

    // Create f(x)=mx+b form for both lines. Use this->start as the origin of
    // the coordinate system.
    float m1 = line.y / line.x;
    float m2 = other.line.y / other.line.x;
    float b2 = m2 * (start.x - other.start.x) + other.start.y - start.y;

    if (std::abs(m1 - m2) < 0.01f) {
        // Lines are parallel (or equivalent, but that should not happen,
        // realistically). Still check with the following assert.
        SDL_assert(b2 != 0.0f);
        return false;
    }

    float intersection_x = b2 / (m1 - m2) + start.x;

    float t1 = (intersection_x - start.x) / line.x;
    float t2 = (intersection_x - other.start.x) / other.line.x;

    if (!isfinite(t1) || !isfinite(t2)) {
        return false;
    }

    if (t1 > 1.0f || t1 < 0.0f || t2 > 1.0f || t2 < 0.0f) {
        return false;
    }

    if (out_t) {
        *out_t = t1;
    }

    return true;
}

bool LineCollider::intersects(const CircleCollider& other) const noexcept {
    if (glm::length(line) + other.radius <
        glm::length(start - other.position)) {
        // Too far away, they can not collide.
        return false;
    }

    glm::vec2 circle_pos_relative_to_line = other.position - start;
    // t * line is the point on the line that is closest to the circle's center
    float t =
        glm::dot(circle_pos_relative_to_line, line) / glm::dot(line, line);

    if (t < 0.0f) {
        // The circle is in the wrong direction relative to the line
        return false;
    }

    glm::vec2 closest_point = t * line;
    float distance = glm::length(closest_point - circle_pos_relative_to_line);

    if (distance > other.radius) {
        // Too far away
        return false;
    }

    return true;
}

const CollisionData
find_first_collision_sweep_prune(const CircleCollider& circle,
                                 const glm::vec2 move,
                                 const std::list<BoxCollider>& boxes) {

    // All colliders that are outside the culling box won't collide, so we don't
    // have to care about them
    BoxCollider culling_box;
    {
        glm::vec2 half_move = move * 0.5f;
        culling_box.position = circle.position + half_move;
        culling_box.half_ext.x = std::abs(half_move.x) + circle.radius + 5.0f;
        culling_box.half_ext.y = std::abs(half_move.y) + circle.radius + 5.0f;
    }

    std::vector<const BoxCollider*> candidates;
    for (const auto& box : boxes) {
        if (culling_box.intersects(box)) {
            candidates.push_back(&box);
        }
    }

    if (move == glm::vec2(0.0f)) {

        // @CLEANUP: Commenting this out does not seem to result in bugs, but it
        // probably should not fire anyway for the system to be robust.
        //#ifdef _DEBUG
        //        auto coll_iter = std::find_if(
        //            candidates.begin(), candidates.end(),
        //            [&circle](const auto& b) { return circle.intersects(*b);
        //            });
        //
        //        if (coll_iter != candidates.end()) {
        //            SDL_TriggerBreakpoint();
        //            circle.intersects(**coll_iter);
        //        }
        //#endif // DEBUG

        return {glm::vec2(0.0f), Direction::NONE};
    }

    glm::vec2 normals[2] = {glm::vec2(-move.y, move.x),
                            glm::vec2(move.y, -move.x)};

    LineCollider sweep_collider_lines[2] = {
        {circle.position + glm::normalize(normals[0]) * circle.radius, move},
        {circle.position + glm::normalize(normals[1]) * circle.radius, move},
    };

    float first_collision_time = 1.0f;
    Direction first_collision_direction = NONE;

    // Find collisions with the collider lines
    for (const auto cand : candidates) {
        float collision_time = 1.0f;
        Direction hit_side;

        for (const auto& line : sweep_collider_lines) {
            if (line.intersects(*cand, &collision_time, &hit_side) &&
                collision_time < first_collision_time) {
                SDL_assert(collision_time >= 0.0f);

                first_collision_time = collision_time;
                if (hit_side == UP) {
                    first_collision_direction = DOWN;
                } else if (hit_side == DOWN) {
                    first_collision_direction = UP;
                } else if (hit_side == LEFT) {
                    first_collision_direction = RIGHT;
                } else if (hit_side == RIGHT) {
                    first_collision_direction = LEFT;
                } else {
                    SDL_TriggerBreakpoint();
                }
            }
        }
    }

    // Some part of the circle might still collide with something at the end
    // point, so let's check for that
    const CircleCollider circle_after_move = {
        circle.position + move * first_collision_time, circle.radius};

    for (const auto cand : candidates) {
        if (circle_after_move.intersects(*cand)) {
            glm::vec2 overlap;

            overlap.x =
                (circle_after_move.radius + cand->half_ext.x) -
                std::abs(circle_after_move.position.x - cand->position.x);
            SDL_assert(overlap.x > 0.0f);

            overlap.y =
                (circle_after_move.radius + cand->half_ext.y) -
                std::abs(circle_after_move.position.y - cand->position.y);
            SDL_assert(overlap.y > 0.0f);
            // SDL_assert(overlap.x <= std::abs(move.x) || overlap.y <=
            // std::abs(move.y));

            SDL_assert(move != glm::vec2(0.0f));

            glm::vec2 collision_time = glm::vec2(-0.1f);
            if (move.x != 0.0f) {
                collision_time.x = 1.0f - overlap.x / std::abs(move.x);
            }
            if (move.y != 0.0f) {
                collision_time.y = 1.0f - overlap.y / std::abs(move.y);
            }

            SDL_assert(collision_time.x <= 1.0f && collision_time.y <= 1.0f);
            if (collision_time.x <= -0.1f && collision_time.y <= -0.1f) {
                continue;
            }

            // Choose the later time because that's when the circle starts
            // touching the candidate box, so it's the furthest it can move
            if (collision_time.x > collision_time.y) {
                if (collision_time.x < first_collision_time) {
                    first_collision_time = collision_time.x;
                    if (move.x > 0.0f) {
                        first_collision_direction = RIGHT;
                    } else {
                        first_collision_direction = LEFT;
                        SDL_assert(move.x != 0.0f);
                    }
                }
            } else { // colision_times.y > collision_time.x
                if (collision_time.y < first_collision_time) {
                    first_collision_time = collision_time.y;
                    if (move.y > 0.0f) {
                        first_collision_direction = UP;
                    } else {
                        first_collision_direction = DOWN;
                        SDL_assert(move.y != 0.0f);
                    }
                }
            }
        }
    }

    CollisionData result;
    result.move_until_collision = move * first_collision_time;
    result.time = first_collision_time;
    result.direction = first_collision_direction;

    // NOTE: These 1.0f margins seem to be pretty large, but without them we hit
    // assertions (due to floating point imprecision?)
    if (first_collision_direction == UP) {
        result.move_until_collision.y -= 2.0f;
    } else if (first_collision_direction == DOWN) {
        result.move_until_collision.y += 2.0f;
    } else if (first_collision_direction == LEFT) {
        result.move_until_collision.x += 2.0f;
    } else if (first_collision_direction == RIGHT) {
        result.move_until_collision.x -= 2.0f;
    }

#ifdef _DEBUG
    CircleCollider after_move = {circle.position + result.move_until_collision,
                                 circle.radius};
    for (const auto& coll : candidates) {
        if (after_move.intersects(*coll)) {
            // SDL_TriggerBreakpoint();
            after_move.intersects(*coll);
        }
    }
#endif

    return result;
}

const BallisticMoveResult
get_ballistic_move_result(const CircleCollider& coll, const glm::vec2 velocity,
                          const float delta_time,
                          const std::list<BoxCollider>& level, float rebound,
                          const size_t max_collision_iterations) {

    glm::vec2 move = velocity * delta_time;
    CollisionData collision;

    collision = find_first_collision_sweep_prune(coll, move, level);

    // NOTE: This condition could be removed and we would still have the same
    // result, but it seems more clear (and efficient) to have it.
    if (collision.direction == NONE) {
        return {coll.position + move, velocity};
    }

    glm::vec2 updated_velocity = velocity;
    CircleCollider updated_collider = coll;
    float remaining_time = delta_time;

    for (size_t num_iterations = 1; num_iterations < max_collision_iterations;
         ++num_iterations) {

        remaining_time -= remaining_time * collision.time;
        updated_collider.position += collision.move_until_collision;

        if (collision.direction == NONE) {
            return {updated_collider.position, updated_velocity};

        } else if (collision.direction == LEFT ||
                   collision.direction == RIGHT) {
            updated_velocity.x *= -rebound;
        } else {
            SDL_assert(collision.direction == UP ||
                       collision.direction == DOWN);
            updated_velocity.y *= -rebound;
        }

        collision = find_first_collision_sweep_prune(
            updated_collider, updated_velocity * remaining_time, level);
    }
    SDL_TriggerBreakpoint();
    return {};
}