#pragma once
#include "pch.h"

struct Texture {
    GLuint id;
    GLuint w, h;
    glm::vec2 dimensions;
};

Texture loadTexture(const char *path);