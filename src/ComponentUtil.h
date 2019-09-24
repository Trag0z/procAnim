#pragma once
#include "Components.h"
#include <gl/glew.h>

SpriteRenderer createSpriteRenderer() {
    SpriteRenderer ret;

    GLfloat vertices[] = {// vec3 pos, vec3 color, vec2 uvCoords
                          0.5f,  0.5f,  0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
                          0.5f,  -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,
                          -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
                          -0.5f, 0.5f,  0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f};

    uint indices[] = {0, 1, 3, 1, 2, 3};

    GLuint vbo, ebo;

    glGenVertexArrays(1, &ret.vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);

    glBindVertexArray(ret.vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices,
                 GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat),
                          (GLvoid *)0);
    glEnableVertexAttribArray(0);
    // color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat),
                          (GLvoid *)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);
    // uvCoord attribute
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat),
                          (GLvoid *)(6 * sizeof(GLfloat)));
    glEnableVertexAttribArray(2);

    // Reset vertex array binding for error safety
    // glBindVertexArray(0);
    // glBindBuffer(GL_ARRAY_BUFFER, 0);

    return ret;
}

FlatRenderer createFlatRenderer() {
    FlatRenderer ret;

    GLuint vbo;
    GLfloat vertices[] = {0.5f, -0.5f, 0.0f, -0.5f, -0.5f,
                          0.0f, 0.0f,  0.5f, 0.0f};

    glGenVertexArrays(1, &ret.vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(ret.vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat),
                          (GLvoid *)0);
    // NOTE: maybe do this while rendering for different array types?
    glEnableVertexAttribArray(0);

    // Reset array binding for error safety
    // glBindBuffer(GL_ARRAY_BUFFER, 0);
    // glBindVertexArray(0);

    return ret;
}