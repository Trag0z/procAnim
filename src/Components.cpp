#pragma once
#include "pch.h"
#include "Components.h"

GLuint SpriteRenderer::shaderID;
GLuint SpriteRenderer::modelMatrixLocation;
GLuint SpriteRenderer::projectionMatrixLocation;

U32 GamepadInput::numGamepads = 0;

void SpriteRenderer::init(GLuint _shaderID) {
    SDL_assert_always(_shaderID != -1);
    SpriteRenderer::shaderID = _shaderID;
    SpriteRenderer::modelMatrixLocation =
        glGetUniformLocation(_shaderID, "model");
    SpriteRenderer::projectionMatrixLocation =
        glGetUniformLocation(_shaderID, "projection");
}