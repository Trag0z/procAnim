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

    vao.init(indices, 6, (DebugShaderVertex*)vertices, 4);
}

void BoxCollider::render(const RenderData& render_data) {
    glUseProgram(render_data.debug_shader.id);

    model = glm::translate(glm::mat4(1.0f), glm::vec3(pos, 0.0f));
    glUniformMatrix4fv(render_data.debug_shader.model_loc, 1, GL_FALSE,
                       value_ptr(model));
    glUniform4f(render_data.debug_shader.color_loc, 1.0f, 0.5f, 0.2f,
                1.0f); // ugly orange-ish

    vao.draw(GL_TRIANGLES);
}