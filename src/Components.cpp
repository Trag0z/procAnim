#pragma once
#include "pch.h"
#include "Components.h"

GLuint SpriteRenderer::shaderID;
GLuint SpriteRenderer::modelMatrixLocation;
GLuint SpriteRenderer::projectionMatrixLocation;

GLuint FlatRenderer::shaderID;
GLint FlatRenderer::colorLocation;

U32 GamepadInput::numGamepads = 0;

void Mesh::init() {
    Mesh m = Mesh::create({{{1.0f, -1.0f, 0.0f}, {1.0f, 0.0f}},
                                    {{-1.0f, 1.0f, 0.0f}, {0.0f, 1.0f}},
                                    {{-1.0f, -1.0f, 0.0f}, {0.0f, 0.0f}},
                                    {{1.0f, 1.0f, 0.0f}, {1.0f, 1.0f}}},
                                   {0, 1, 2, 0, 3, 1});
    simpleMesh.vao = m.vao;
    simpleMesh.numIndices = m.numIndices;
}

Mesh Mesh::create(std::vector<Vertex> vertices, std::vector<uint> indices) {
    Mesh result;
    result.numIndices = static_cast<GLuint>(indices.size());
    GLuint vbo, ebo;

    glGenVertexArrays(1, &result.vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);

    glBindVertexArray(result.vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * vertices.size(), vertices.data(),
                 GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint) * indices.size(),
                 indices.data(), GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          reinterpret_cast<void*>(0));
    glEnableVertexAttribArray(0);
    // uvCoord attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          reinterpret_cast<void*>(offsetof(Vertex, uvCoord)));
    glEnableVertexAttribArray(1);

    // Reset vertex array binding for error safety
    glBindVertexArray(0);

	return result;
}

Mesh Mesh::simple() { return {simpleMesh.vao, simpleMesh.numIndices}; }

void SpriteRenderer::init(GLuint _shaderID) {
    SDL_assert_always(_shaderID != -1);
    SpriteRenderer::shaderID = _shaderID;
    SpriteRenderer::modelMatrixLocation =
        glGetUniformLocation(_shaderID, "model");
    SpriteRenderer::modelMatrixLocation =
        glGetUniformLocation(_shaderID, "projection");
}

void FlatRenderer::init(GLuint _shaderID) {
    FlatRenderer::shaderID = _shaderID;
    FlatRenderer::colorLocation = glGetUniformLocation(shaderID, "outcolor");
}