#pragma once
#include "pch.h"
#include "Shaders.h"
#include "Texture.h"

class Renderer {
    glm::vec2 window_size_ = {1920.0f, 1200.0f};
    glm::vec2 camera_center_ = {0.0f, 0.0f};
    float zoom_factor_ = 1.0f;

  public:
    void update_camera(const glm::vec2& center, float zoom_factor = 0.0f);

    DebugShader debug_shader;
    TexturedShader textured_shader;
    RiggedShader rigged_shader;
    RiggedDebugShader rigged_debug_shader;
    BoneShader bone_shader;

    void init();

    glm::vec2 window_size() const noexcept;
    glm::vec2 camera_position() const noexcept;
    glm::vec2 camera_center() const noexcept;
    float zoom_factor() const noexcept;

    bool draw_body = false;
    bool draw_limbs = true;
    bool draw_bones = false;
    bool draw_wireframe = false;
    bool draw_colliders = true;
    bool draw_leg_splines = true;
};