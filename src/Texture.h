#pragma once
#include <gl/glew.h>
#include <sdl/SDL_image.h>

struct Texture {
    GLuint id;
    GLuint w, h;
};

Texture loadTexture(const char *path);