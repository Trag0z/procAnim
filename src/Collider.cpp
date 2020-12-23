#pragma once
#include "pch.h"
#include "Collider.h"

bool BoxCollider::encloses_point(glm::vec2 point) const noexcept {
    return point.x > position.x - half_ext.x &&
           point.x < position.x + half_ext.x &&
           point.y > position.y - half_ext.y &&
           point.y < position.y + half_ext.y;
}

bool BoxCollider::intersects(const BoxCollider& other) const noexcept {
    SDL_assert(half_ext.x >= 0.0f && half_ext.y >= 0.0f);
    SDL_assert(other.half_ext.x >= 0.0f && other.half_ext.y >= 0.0f);

    return (std::abs(position.x - other.position.x) <
            half_ext.x + other.half_ext.x) &&
           (std::abs(position.y - other.position.y) <
            half_ext.y + other.half_ext.y);
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

glm::mat3 BoxCollider::calculate_model_matrix() const noexcept {
    glm::mat3 model = glm::translate(glm::mat3(1.0f), position);
    return glm::scale(model, half_ext);
}

bool CircleCollider::intersects(const BoxCollider& other) const noexcept {
    SDL_assert(radius >= 0.0f);
    SDL_assert(other.half_ext.x >= 0.0f && other.half_ext.y >= 0.0f);

    return std::abs(position.x - other.position.x) <=
               radius + other.half_ext.x &&
           std::abs(position.y - other.position.y) <= radius + other.half_ext.y;
}

const CollisionData
find_first_collision_sweep_prune(const CircleCollider& circle,
                                 const glm::vec2 move,
                                 const std::list<BoxCollider> boxes) {
    if (move == glm::vec2(0.0f)) {
        // TODO: Just check for collisions with the circle?
        return {glm::vec2(0.0f), CollisionData::NONE};
    }

    // Cull all boxes that are too far away from the circle's trajectory
    BoxCollider culling_box;
    {
        glm::vec2 half_move = move * 0.5f;
        culling_box.position = circle.position + half_move;
        culling_box.half_ext.x = std::abs(half_move.x) + circle.radius;
        culling_box.half_ext.y = std::abs(half_move.y) + circle.radius;
    }

    std::vector<const BoxCollider*> candidates;
    for (const auto& box : boxes) {
        if (culling_box.intersects(box)) {
            candidates.push_back(&box);
        }
    }

    float collision_time = 1.1f;

    CollisionData result = {move, CollisionData::NONE};
    // Find first collision along x-axis
    // @OPTIMIZATION: Sorting the array vs. just iterating over all candidates
    // and choosing the one with the smallest collision_time?
    if (move.x > 0.0f) { // Moving right
        std::sort(candidates.begin(), candidates.end(),
                  [](const auto a, const auto b) {
                      return a->left_edge() < b->left_edge();
                  });

        for (const auto c : candidates) {
            glm::vec2 circle_collision_pos = {c->left_edge() - circle.radius,
                                              0.0f};

            collision_time =
                (circle_collision_pos.x - circle.position.x) / move.x;
            if (collision_time < 0.0f)
                continue;

            circle_collision_pos =
                circle.position + collision_time * move;

            if (std::abs(circle_collision_pos.y - c->position.y) <=
                circle.radius + c->half_ext.y) {

                result = {move * collision_time, CollisionData::RIGHT};
                break; // Since the list is sorted, the collision has to be
                       // the first on the path and we don't have to look at the
                       // others
            }
        }
    } else if (move.x < 0.0f) { // Moving left
        std::sort(candidates.begin(), candidates.end(),
                  [](const auto a, const auto b) {
                      return a->right_edge() < b->right_edge();
                  });

        for (const auto c : candidates) {
            glm::vec2 circle_collision_pos = {c->right_edge() + circle.radius,
                                              0.0f};

            collision_time =
                (circle_collision_pos.x - circle.position.x) / move.x;
            if (collision_time < 0.0f)
                continue;

            circle_collision_pos =
                circle.position + collision_time * move;

            if (std::abs(circle_collision_pos.y - c->position.y) <=
                circle.radius + c->half_ext.y) {

                result = {move * collision_time, CollisionData::LEFT};
                break; // Since the list is sorted, the collision has to be
                       // the first on the path and we don't have to look at the
                       // others
            }
        }
    }

    // Do the same along the y-axis, replace result if it's earlier than the
    // current result
    if (move.y > 0.0f) { // Moving up
        std::sort(candidates.begin(), candidates.end(),
                  [](const auto a, const auto b) {
                      return a->bottom_edge() < b->bottom_edge();
                  });

        for (const auto c : candidates) {
            glm::vec2 circle_collision_pos = {c->bottom_edge() - circle.radius,
                                              0.0f};

            float new_collision_time =
                (circle_collision_pos.y - circle.position.y) / move.y;
            if (new_collision_time < 0.0f)
                continue;

            if (new_collision_time < collision_time) {
                circle_collision_pos = circle.position + collision_time * move;

                if (std::abs(circle_collision_pos.x - c->position.x) <=
                    circle.radius + c->half_ext.x) {

                    glm::vec2 move_to_collision = move * new_collision_time;
                    result = {move_to_collision, CollisionData::UP};
                    break; // Since the list is sorted, so the collision has to
                           // be the first on the path and we don't have to look
                           // at the others
                }
            }
        }
    } else if (move.y < 0.0f) { // Moving down
        std::sort(candidates.begin(), candidates.end(),
                  [](const auto a, const auto b) {
                      return a->top_edge() < b->top_edge();
                  });

        for (const auto c : candidates) {
            glm::vec2 circle_collision_pos = {c->top_edge() + circle.radius,
                                              0.0f};

            float new_collision_time =
                (circle_collision_pos.y - circle.position.y) / move.y;
            if (new_collision_time < 0.0f)
                continue;

            if (new_collision_time < collision_time) {
                circle_collision_pos = circle.position + collision_time * move;

                if (std::abs(circle_collision_pos.x - c->position.x) <=
                    circle.radius + c->half_ext.x) {

                    glm::vec2 move_to_collision = move * new_collision_time;
                    result = {move_to_collision, CollisionData::DOWN};
                    break; // Since the list is sorted, so the collision has to
                           // be the first on the path and we don't have to look
                           // at the others
                }
            }
        }
    }

    return result;

    // NOTE: This would be kinda nice, but we have to check if the result is
    // the past-the-end iterator before dereferencing, which makes the
    // correct code ugly again. result =
    //     *std::find_if(candidates.begin(), candidates.end(),
    //                   [circle](const auto c) { return
    //                   circle.intersects(c);
    //                   });
}