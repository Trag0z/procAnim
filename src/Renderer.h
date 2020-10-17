#pragma once
#include "pch.h"
#include "Shaders.h"
#include "Texture.h"

class Renderer {
    glm::vec2 window_size_ = {1920.0f, 1200.0f};
    glm::vec2 camera_center_ = {0.0f, 0.0f};

  public:
    void update_camera(const glm::vec2& center);

    DebugShader debug_shader;
    TexturedShader textured_shader;
    RiggedShader rigged_shader;
    RiggedDebugShader rigged_debug_shader;

    void init();

    glm::vec2 window_size() const noexcept;
    glm::vec2 camera_position() const noexcept;
    glm::vec2 camera_center() const noexcept;

    bool draw_models = true;
    bool draw_bones = true;
    bool draw_wireframes = false;
    bool draw_arm_splines = false;
    bool draw_leg_splines = true;
    bool draw_pelvis_spline = true;
};