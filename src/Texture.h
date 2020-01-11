#pragma once
#include <gl/glew.h>
#include <glm/glm.hpp>
#include <sdl/SDL_image.h>

struct Texture {
    GLuint id;
    GLuint w, h;
    glm::vec2 dimensions;
};

Texture loadTexture(const char *path);