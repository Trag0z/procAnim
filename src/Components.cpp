#pragma once
#include "pch.h"
#include "Components.h"

U32 GamepadInput::numGamepads = 0;

void RenderData::init(GLuint simpleShaderId, GLuint riggedShaderId) {
    SDL_assert_always(simpleShaderId != -1 && riggedShaderId != -1);
    simpleShader.id = simpleShaderId;
    simpleShader.modelMatrixLoc = glGetUniformLocation(simpleShaderId, "model");
    simpleShader.projectionMatrixLoc =
        glGetUniformLocation(simpleShaderId, "projection");

#ifndef SHADER_DEBUG
    riggedShader.id = riggedShaderId;
    riggedShader.modelMatrixLoc = glGetUniformLocation(riggedShaderId, "model");
    riggedShader.projectionMatrixLoc =
        glGetUniformLocation(riggedShaderId, "projection");
    riggedShader.bonesLoc = glGetUniformLocation(riggedShaderId, "bones");
#endif
}

glm::vec2 IHasPosition::worldPos() const { return transform->pos + pos; }

float BoxCollider::top() const { return pos.y - halfExt.y; }
float BoxCollider::bot() const { return pos.y + halfExt.y; }
glm::vec2 BoxCollider::topRight() const { return worldPos() - halfExt; }
glm::vec2 BoxCollider::botLeft() const { return worldPos() + halfExt; }