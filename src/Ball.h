#pragma once
#include <list>
#include "Collider.h"
#include "Entity.h"
#include "rendering/Texture.h"
#include "Audio.h"

class Renderer;
class ConfigManager;

class Ball : Entity {
    glm::vec2 velocity;
    float rotation = 0.0f;
    float rotation_speed = 0.0f;
    Circle collider_;
    bool grounded = false;

    Texture texture;

    struct Trajectory {
        static const GLuint NUM_VERTICES = 120;
        static const Color COLOR;
        VertexArray<DebugShader::Vertex> vao;
        std::array<DebugShader::Vertex, NUM_VERTICES> vertices;
    } trajectory;

    static float REBOUND;
    static float RADIUS;
    static float ROLLING_FRICTION;
    static float GRAVITY;
    static float ROLLING_ROTATION_SPEED;

  public:
    float freeze_duration = 0.0f;

    void init(glm::vec2 position, const char* texture_path);
    void update(const float delta_time, const std::list<AABB>& level,
                AudioManager& audio_manager, bool trajectory);
    void render(const Renderer& renderer) const;
    bool display_debug_ui();

    void set_velocity(glm::vec2 velocity);

    const Circle collider() const;

    friend ConfigManager;
};