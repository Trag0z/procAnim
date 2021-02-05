#pragma once
#include "pch.h"
#include "Ball.h"
#include "rendering/Renderer.h"

void Ball::init(glm::vec2 position, float radius, const char* texture_path) {
    Entity::init(position, glm::vec2(radius));
    collider_ = {glm::vec2(0.0f), radius};
    texture.load_from_file(texture_path);
}

void Ball::update(const float gravity, const float delta_time,
                  const std::list<BoxCollider>& level) {

    BallisticMoveResult move_result = get_ballistic_move(
        collider_.local_to_world_space(*this), velocity, delta_time, level);

    position_ = move_result.new_position;
    velocity = move_result.new_velocity;
    velocity.y -= gravity;

    update_model_matrix();
}

void Ball::render(const Renderer& renderer) const {
    renderer.textured_shader.use();
    renderer.textured_shader.set_model(&model);
    renderer.textured_shader.set_texture(texture);
    renderer.textured_shader.DEFAULT_VAO.draw(GL_TRIANGLES);
}

bool Ball::display_debug_ui() {
    using namespace ImGui;
    bool keep_open = true;

    Begin("Ball", &keep_open);

    PushItemWidth(100);
    DragFloat2("position", value_ptr(position_));
    DragFloat2("velocity", value_ptr(velocity));

    if (DragFloat("radius", &collider_.radius)) {
        scale = glm::vec2(collider_.radius);
        update_model_matrix();
    }

    PopItemWidth();
    End();

    return keep_open;
}

void Ball::set_velocity(glm::vec2 velo) { velocity = velo; }

const CircleCollider& Ball::collider() const { return collider_; }