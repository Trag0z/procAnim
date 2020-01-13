#pragma once
#include "pch.h"
#include "Components.h"

U32 GamepadInput::numGamepads = 0;

void RenderData::init(GLuint simpleShaderId, GLuint riggedShaderId) {
    SDL_assert_always(simpleShaderId != -1 && riggedShaderId != -1);
    simpleShader.id = simpleShaderId;
    simpleShader.modelMatrix = glGetUniformLocation(simpleShaderId, "model");
    simpleShader.projectionMatrix =
        glGetUniformLocation(simpleShaderId, "projection");

    riggedShader.id = riggedShaderId;
    riggedShader.modelMatrix = glGetUniformLocation(riggedShaderId, "model");
    riggedShader.projectionMatrix =
        glGetUniformLocation(riggedShaderId, "projection");
    riggedShader.bones = glGetUniformLocation(riggedShaderId, "bones");
}