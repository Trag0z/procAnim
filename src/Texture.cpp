#pragma once
#include "pch.h"
#include "Texture.h"

Texture loadTexture(const char *path) {
    SDL_Surface *img = IMG_Load(path);
    SDL_assert(img);

    Texture ret;
    ret.w = img->w;
    ret.h = img->h;

    ret.dimensions = {static_cast<float>(ret.w), static_cast<float>(ret.h)};

    glGenTextures(1, &ret.id);
    glBindTexture(GL_TEXTURE_2D, ret.id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, ret.w, ret.h, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, img->pixels);
    // NOTE: Is this actually useful?
    glGenerateMipmap(GL_TEXTURE_2D);
    SDL_FreeSurface(img);

    // Set Texture wrap and filter modes
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // Unbind texture
    glBindTexture(GL_TEXTURE_2D, 0);

    return ret;
}