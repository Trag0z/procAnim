#pragma once
#include "pch.h"

struct Texture {
    GLuint id;
    GLuint w, h;
    glm::vec2 dimensions;

    void load_from_file(const char* path);
};