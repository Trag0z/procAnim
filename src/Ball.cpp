#pragma once
#include "Ball.h"
#include "rendering/Renderer.h"
#include "Util.h"
#include "CollisionDetection.h"
#include <glm/ext.hpp>
#include <imgui/imgui.h>
#include <glm/glm.hpp>

float Ball::REBOUND = 1.0f;
float Ball::RADIUS = 50.0f;
float Ball::ROLLING_FRICTION = 1.5f;
float Ball::ROLLING_ROTATION_SPEED = 1.0f;
float Ball::GRAVITY = 1.0f;

void Ball::init(glm::vec2 position, const char* texture_path) {
    Entity::init(position, glm::vec2(RADIUS));
    collider_ = {glm::vec2(0.0f), 1.0f};
    texture.load_from_file(texture_path);
}

void Ball::update(const float delta_time, const MemArena<AABB>& level) {
    if (RADIUS != scale.x) {
        SDL_assert(scale.x == scale.y);
        scale = glm::vec2(RADIUS);
    }

    BallisticMoveResult move_result =
        get_ballistic_move_result(collider_.local_to_world_space(*this),
                                  velocity, delta_time, level, REBOUND);

    position_ = move_result.new_position;
    velocity = move_result.new_velocity;

    if (grounded) {
        velocity.x *= 1.0f / ROLLING_FRICTION;
        velocity.y = 0.0f;

        rotation_speed = -ROLLING_ROTATION_SPEED * velocity.x;
    } else {
        if (move_result.last_hit_diretcion == Direction::DOWN &&
            move_result.new_velocity.y <= GRAVITY * delta_time * 2.0f) {

            velocity.y = 0.0f;
            grounded = true;

            // position_.y = move_result.last_hit_object->max(1) + RADIUS
            // + 2.0f;

#if _DEBUG
            update_model_matrix();
            {
                Circle global_circle = collider_.local_to_world_space(*this);
                for (const auto& box : level) {
                    SDL_assert(!test_circle_AABB(global_circle, box));
                }
            }
#endif
        } else {
            velocity.y -= GRAVITY;
        }

        if (move_result.last_hit_diretcion == Direction::UP ||
            move_result.last_hit_diretcion == Direction::DOWN) {
            rotation_speed = -ROLLING_ROTATION_SPEED * velocity.x;
        } else if (move_result.last_hit_diretcion == Direction::RIGHT ||
                   move_result.last_hit_diretcion == Direction::RIGHT) {
            rotation_speed = -ROLLING_ROTATION_SPEED * velocity.y;
        }
    }
    update_model_matrix();

    rotation += rotation_speed;
    if (rotation > 2.0f * PI) {
        rotation -= 2.0f * PI;
    } else if (rotation < 2.0f * PI) {
        rotation += 2.0f * PI;
    }
}

void Ball::render(const Renderer& renderer) const {
    renderer.textured_shader.use();

    glm::mat3 rotated_model = glm::rotate(model, rotation);
    renderer.textured_shader.set_model(&rotated_model);

    renderer.textured_shader.set_texture(texture);
    renderer.textured_shader.DEFAULT_VAO.draw(GL_TRIANGLES);
}

bool Ball::display_debug_ui() {
    using namespace ImGui;
    bool keep_open = true;

    Begin("Ball", &keep_open);

    PushItemWidth(150);
    DragFloat2("position", value_ptr(position_));
    DragFloat2("velocity", value_ptr(velocity));
    DragFloat("rotation", &rotation, 0.1f);

    PopItemWidth();
    End();

    return keep_open;
}

void Ball::set_velocity(glm::vec2 velo) {
    velocity = velo;
    grounded = false;
}

const Circle Ball::collider() const {
    return collider_.local_to_world_space(*this);
}