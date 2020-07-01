#pragma once
#include "pch.h"
#include "Shaders.h"

namespace {
glm::mat3 calculate_camera(float left, float right, float top, float bottom) {
    glm::mat3 ret =
        glm::scale(glm::mat3(1.0f),
                   glm::vec2(2.0f / (left + right), 2.0f / (top + bottom)));
    ret = glm::translate(
        ret, glm::vec2((left + right) * -0.5f, (top + bottom) * -0.5f));
    return ret;
}
} // namespace

class Renderer {
    glm::ivec2 window_size_ = {1920, 1080};

  public:
    const glm::mat3 camera = calculate_camera(0.0f, 1920.0f, 0.0f, 1080.0f);
    RiggedShader rigged_shader;
    DebugShader debug_shader;

    void init();

    inline glm::ivec2 window_size() const { return window_size_; }

    bool draw_models = true;
    bool draw_bones = true;
    bool draw_wireframes = false;
    bool draw_splines = false;
};