#pragma once
#include "pch.h"
#include "VertexArray.h"
#include "Renderer.h"

struct BoxCollider {
    glm::vec2 pos;
    glm::vec2 half_ext;

    glm::mat3 model;

    glm::vec2 vertices[4];

    VertexArray<DebugShader::Vertex> vao;

    BoxCollider() {}
    BoxCollider(glm::vec2 pos, glm::vec2 half_ext);

    void render(const Renderer& renderer);
};