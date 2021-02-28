#pragma once
#include "Shaders.h"
#include "Texture.h"
#include "../ConfigManager.h"
#include <PerlinNoise.hpp>

class Game;

class Renderer {
    glm::vec2 window_size_ = {1920.0f, 1080.0f};
    glm::vec2 camera_center_ = {0.0f, 0.0f};
    float zoom_factor_ = 1.0f;

    struct {
        float intensity;
        float duration;
        float speed;

        float noise_pos = 0.0f;
        const siv::BasicPerlinNoise<float> noise[2] = {
            siv::BasicPerlinNoise<float>(1337),
            siv::BasicPerlinNoise<float>(5345)};
    } screen_shake;

  public:
    DebugShader debug_shader;
    TexturedShader textured_shader;
    RiggedShader rigged_shader;
    RiggedDebugShader rigged_debug_shader;
    BoneShader bone_shader;
    TrailShader trail_shader;

    void init();
    void update(float delta_time);
    void shake_screen(float intensity, float duration, float speed);

    glm::vec2 window_size() const noexcept;
    glm::vec2 camera_position() const noexcept;
    glm::vec2 camera_center() const noexcept;
    float zoom_factor() const noexcept;

    glm::vec2 screen_to_world_space(glm::vec2 screen_pos) const noexcept;

    bool draw_body = false;
    bool draw_limbs = true;
    bool draw_bones = false;
    bool draw_wireframes = false;
    bool draw_colliders = true;
    bool draw_leg_splines = true;
    bool draw_weapon_trails = true;
    bool draw_ball_trajectory = true;

    friend ConfigManager;
    friend Game;
};