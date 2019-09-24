#pragma once
#include "Texture.h"

void loadTexture(Texture &out, const char *path) {
    SDL_Surface *img = IMG_Load(path);

    glGenTextures(1, &out.id);
    out.width = img->w;
    out.height = img->h;

    glBindTexture(GL_TEXTURE_2D, out.id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, out.width, out.height, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, img->pixels);

    // Set Texture wrap and filter modes
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // Unbind texture
    glBindTexture(GL_TEXTURE_2D, 0);
}