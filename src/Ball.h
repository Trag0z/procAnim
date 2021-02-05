#pragma once
#include "pch.h"
#include "Collider.h"
#include "Entity.h"
#include "rendering/Texture.h"

class Renderer;

class Ball : Entity {
    glm::vec2 velocity;
    float radius;
    CircleCollider collider_;

    Texture texture;

  public:
    void init(glm::vec2 position, float radius_, const char* texture_path);
    void update(const float gravity, const float delta_time,
                const std::list<BoxCollider>& level);
    void render(const Renderer& renderer) const;
    bool display_debug_ui();

    void set_velocity(glm::vec2 velocity);

    const CircleCollider collider() const;
};