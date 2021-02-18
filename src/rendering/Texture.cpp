#pragma once
#include "Texture.h"
#include <sdl/SDL_image.h>
#include <gl/glew.h>

void Texture::load_from_file(const char* path) {
    SDL_Surface* img = IMG_Load(path);
    SDL_assert(img);

    w = img->w;
    h = img->h;

    dimensions = {static_cast<float>(w), static_cast<float>(h)};

    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 img->pixels);
    SDL_FreeSurface(img);

    // Set Texture wrap and filter modes
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Unbind texture
    glBindTexture(GL_TEXTURE_2D, 0);
}