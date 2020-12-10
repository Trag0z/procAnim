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

    bool encloses_point(glm::vec2 point) const noexcept;

    float left_edge() const noexcept;
    float right_edge() const noexcept;
    float top_edge() const noexcept;
    float bottom_edge() const noexcept;
};