#pragma once
#include "pch.h"
#include "VertexArrayData.h"

struct BoxCollider {
    glm::vec2 pos;
    glm::vec2 half_ext;

    glm::mat4 model;

    std::vector<glm::vec4> vertices;

    VertexArrayData<DebugShaderVertex> vao;
    std::vector<DebugShaderVertex> shader_vertices;

    BoxCollider() {}
    BoxCollider(glm::vec2 pos, glm::vec2 half_ext);
};