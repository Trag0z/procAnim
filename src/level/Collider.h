#pragma once
#include "../pch.h"
#include "../VertexArray.h"
#include "../Color.h"

class Renderer;

struct BoxCollider {
    glm::vec2 position;
    glm::vec2 half_ext;

    glm::mat3 model;

    static const Color DEFAULT_COLOR;
    Color color = DEFAULT_COLOR;

    DebugShader::Vertex vertices[4];
    VertexArray<DebugShader::Vertex> vao;

    BoxCollider(glm::vec2 pos, glm::vec2 half_ext);

    void update_vertex_data();
    void render(const Renderer& renderer) const;

    bool is_inside_rect(glm::vec2 point) const noexcept;
};