#pragma once
#include <list>
#include "Collider.h"
#include "Entity.h"
#include "rendering/Texture.h"

class Renderer;
class ConfigManager;

class Ball : Entity {
    glm::vec2 velocity;
    float rotation = 0.0f;
    float rotation_speed = 0.0f;
    Circle collider_;
    bool grounded = false;

    Texture texture;

    static float REBOUND;
    static float RADIUS;
    static float ROLLING_FRICTION;
    static float GRAVITY;
    static float ROLLING_ROTATION_SPEED;

  public:
    void init(glm::vec2 position, const char* texture_path);
    void update(const float delta_time, const MemArena<AABB>& level);
    void render(const Renderer& renderer) const;
    bool display_debug_ui();

    void set_velocity(glm::vec2 velocity);

    const Circle collider() const;

    friend ConfigManager;
};