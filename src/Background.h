#pragma once
#include "pch.h"
#include "Texture.h"
#include "Entity.h"
#include "Shaders.h"

class Renderer;

class Background : Entity {
    glm::vec2 size;
    Texture tex;
    VertexArray<TexturedShader::Vertex> vao;

  public:
    void init(const char* texture_path);
    void render(const Renderer& renderer, glm::vec2 camera_position);
};