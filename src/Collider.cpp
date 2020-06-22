#pragma once
#include "pch.h"
#include "Collider.h"
#include "Game.h"

BoxCollider::BoxCollider(glm::vec2 position, glm::vec2 half_extents) {
    pos = position;
    half_ext = half_extents;

    GLuint indices[6] = {0, 1, 2, 1, 3, 2};
    vertices[0] = {-half_ext.x, half_ext.y, 0.0f, 1.0f};
    vertices[1] = {half_ext.x, half_ext.y, 0.0f, 1.0f};
    vertices[2] = {-half_ext.x, -half_ext.y, 0.0f, 1.0f};
    vertices[3] = {half_ext.x, -half_ext.y, 0.0f, 1.0f};

    vao.init(indices, 6, (DebugShader::Vertex*)vertices, 4);
}

void BoxCollider::render(const Renderer& renderer) {
    renderer.debug_shader.use();

    model = glm::translate(glm::mat4(1.0f), glm::vec3(pos, 0.0f));
    renderer.debug_shader.set_model(&model);

    renderer.debug_shader.set_color(&Colors::ORANGE);

    vao.draw(GL_TRIANGLES);
}