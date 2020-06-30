#pragma once
#include "pch.h"
#include "Collider.h"
#include "Game.h"

BoxCollider::BoxCollider(glm::vec2 position, glm::vec2 half_extents) {
    pos = position;
    half_ext = half_extents;

    GLuint indices[6] = {0, 1, 2, 1, 3, 2};
    vertices[0] = {-half_ext.x, half_ext.y};
    vertices[1] = {half_ext.x, half_ext.y};
    vertices[2] = {-half_ext.x, -half_ext.y};
    vertices[3] = {half_ext.x, -half_ext.y};

    vao.init(indices, 6, (DebugShader::Vertex*)vertices, 4);
}

void BoxCollider::render(const Renderer& renderer) {
    renderer.debug_shader.use();

    model = glm::translate(glm::mat3(1.0f), pos);
    renderer.debug_shader.set_model(&model);

    renderer.debug_shader.set_color(&Colors::ORANGE);

    vao.draw(GL_TRIANGLES);

    glm::vec3 sim_pos = renderer.camera * model * glm::vec3(half_ext, 1.0f);
}