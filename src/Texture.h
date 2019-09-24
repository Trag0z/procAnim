#pragma once
#include <gl/glew.h>
#include <sdl/SDL_image.h>

struct Texture {
    GLuint id;
    GLuint width, height;
};

void loadTexture(Texture &out, const char *path);