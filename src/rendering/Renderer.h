#pragma once
#include "../pch.h"
#include "Shaders.h"
#include "Texture.h"
#include "../ConfigLoader.h"

class Renderer {
    glm::vec2 window_size_ = {1920.0f, 1080.0f};
    glm::vec2 camera_center_ = {0.0f, 0.0f};
    float zoom_factor_ = 1.0f;

  public:
    void update_camera();

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

    glm::vec2 screen_to_world_space(glm::vec2 screen_pos) const noexcept;

    bool draw_body = false;
    bool draw_limbs = true;
    bool draw_bones = false;
    bool draw_wireframes = false;
    bool draw_colliders = true;
    bool draw_leg_splines = true;

    friend ConfigLoader;
};