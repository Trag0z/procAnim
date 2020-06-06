#pragma once
#include "pch.h"
#include "VertexArrayData.h"

struct RenderData;

struct BoxCollider {
    glm::vec2 pos;
    glm::vec2 half_ext;

    glm::mat4 model;

    glm::vec4 vertices[4];

    VertexArrayData<DebugShaderVertex> vao;

    BoxCollider() {}
    BoxCollider(glm::vec2 pos, glm::vec2 half_ext);

    void render(const RenderData& render_data);
};