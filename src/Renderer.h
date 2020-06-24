#pragma once
#include "pch.h"
#include "Shaders.h"

class Renderer {
    const glm::mat4 projection = glm::ortho(
        0.0f, 1920.0f, 0.0f, 1080.0f, -1.0f,
        1.0f); // TODO: Change the order of 3rd and 4th argument so -y is up

    glm::ivec2 window_size_ = {1920, 1080};

  public:
    RiggedShader rigged_shader;
    DebugShader debug_shader;

    void init();

    inline glm::ivec2 window_size() const { return window_size_; }

    bool draw_models = true;
    bool draw_bones = true;
    bool draw_wireframes = false;
    bool draw_splines = false;
};