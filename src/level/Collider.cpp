#pragma once
#include "pch.h"
#include "Collider.h"
#include "../Renderer.h"

Texture BoxCollider::TEXTURE;

BoxCollider::BoxCollider(glm::vec2 position_, glm::vec2 half_extents) {
    position = position_;
    half_ext = half_extents;

    update_model_matrix();
}

void BoxCollider::update_model_matrix() {
    model = glm::translate(glm::mat3(1.0f), position);
    model = glm::scale(model, half_ext);
}

void BoxCollider::render(const Renderer& renderer) const {
    renderer.textured_shader.set_model(&model);
    renderer.textured_shader.DEFAULT_VAO.draw(GL_TRIANGLES);
}

bool BoxCollider::is_inside_rect(glm::vec2 point) const noexcept {
    return point.x > position.x - half_ext.x &&
           point.x < position.x + half_ext.x &&
           point.y > position.y - half_ext.y &&
           point.y < position.y + half_ext.y;
}