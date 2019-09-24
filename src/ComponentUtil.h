#pragma once
#include "Components.h"
#include <gl/glew.h>

SpriteRenderer createSpriteRenderer() {
    SpriteRenderer ret;

    GLuint vbo;
    GLfloat vertices[] = {// Pos      Tex         Pos         Tex
                          0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f,
                          0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
                          1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f};

    glGenVertexArrays(1, &ret.vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(ret.vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat),
                          (GLvoid *)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Reset vertex array binding for error safety
    glBindVertexArray(0);

    return ret;
}

FlatRenderer createFlatRenderer() {
    FlatRenderer ret;

    GLuint vbo;
    GLfloat vertices[] = {0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f};

    glGenVertexArrays(1, &ret.vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(ret.vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat),
                          (GLvoid *)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Reset vertex array binding for error safety
    glBindVertexArray(0);

    return ret;
}