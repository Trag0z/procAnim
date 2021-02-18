#pragma once
#include <glm/glm.hpp>
#include "Types.h"

struct Texture {
    GLuint id;
    GLuint w, h;
    vec2 dimensions;

    void load_from_file(const char* path);
};