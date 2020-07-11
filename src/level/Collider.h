#pragma once
#include "../pch.h"
#include "../VertexArray.h"
#include "../Color.h"
#include "../Texture.h"

class Renderer;

struct BoxCollider {
    glm::vec2 position;
    glm::vec2 half_ext;

    glm::mat3 model;

    static Texture TEXTURE;

    BoxCollider(glm::vec2 pos, glm::vec2 half_ext);

    void update_model_matrix();
    void render(const Renderer& renderer) const;

    bool is_inside_rect(glm::vec2 point) const noexcept;
};