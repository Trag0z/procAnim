#pragma once
#include "pch.h"
#include "Collider.h"
#include "../Renderer.h"

BoxCollider::BoxCollider(glm::vec2 position_, glm::vec2 half_extents) {
    position = position_;
    half_ext = half_extents;

    model = glm::translate(glm::mat3(1.0f), position);

    GLuint indices[6] = {0, 1, 2, 1, 3, 2};
    vertices[0] = {-half_ext.x, half_ext.y};
    vertices[1] = {half_ext.x, half_ext.y};
    vertices[2] = {-half_ext.x, -half_ext.y};
    vertices[3] = {half_ext.x, -half_ext.y};

    vao.init(indices, 6, (DebugShader::Vertex*)vertices, 4);
}

void BoxCollider::update_vertex_data() {
    model = glm::translate(glm::mat3(1.0f), position);

    vertices[0] = {-half_ext.x, half_ext.y};
    vertices[1] = {half_ext.x, half_ext.y};
    vertices[2] = {-half_ext.x, -half_ext.y};
    vertices[3] = {half_ext.x, -half_ext.y};
}

void BoxCollider::render(const Renderer& renderer) const {
    renderer.debug_shader.set_model(&model);
    renderer.debug_shader.set_color(&Colors::ORANGE);
    vao.draw(GL_TRIANGLES);
}

bool BoxCollider::is_inside_rect(glm::vec2 point) const noexcept {
    return point.x > position.x - half_ext.x &&
           point.x < position.x + half_ext.x &&
           point.y > position.y - half_ext.y &&
           point.y < position.y + half_ext.y;
}