#pragma once
#include "pch.h"
#include "Color.h"
#include "Texture.h"
#include "Entity.h"
#include "Level.h"

class Renderer;
class LevelEditor;

class BoxCollider : public Entity {
    glm::vec2 half_ext_;

  public:
    BoxCollider(glm::vec2 position, glm::vec2 half_ext);

    void update_model_matrix();

    void render(const Renderer& renderer) const;

    glm::vec2 half_ext() const;

    bool encloses_point(glm::vec2 point) const noexcept;

    float left_edge() const noexcept;
    float right_edge() const noexcept;
    float top_edge() const noexcept;
    float bottom_edge() const noexcept;

    friend LevelEditor;
};