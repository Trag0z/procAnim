#pragma once
#include "pch.h"
#include "Collider.h"

BoxCollider::BoxCollider(glm::vec2 position, glm::vec2 half_extents) {
    pos = position;
    half_ext = half_extents;

    GLuint indices[6] = {0, 1, 2, 1, 3, 2};
    vertices.reserve(4);
    vertices.emplace_back(-half_ext.x, half_ext.y, 0.0f, 1.0f);
    vertices.emplace_back(half_ext.x, half_ext.y, 0.0f, 1.0f);
    vertices.emplace_back(-half_ext.x, -half_ext.y, 0.0f, 1.0f);
    vertices.emplace_back(half_ext.x, -half_ext.y, 0.0f, 1.0f);

    shader_vertices.reserve(4);
    shader_vertices.push_back({glm::vec4()});
    shader_vertices.push_back({glm::vec4()});
    shader_vertices.push_back({glm::vec4()});
    shader_vertices.push_back({glm::vec4()});

    vao.init(indices, 6, NULL, static_cast<GLuint>(shader_vertices.size()));
}