#pragma once
#include "pch.h"
#include "Shaders.h"
#include "Player.h"

class Player;
struct BoxCollider;

class Renderer {
    glm::vec2 window_size_ = {1920.0f, 1080.0f};

    void update_camera(const glm::vec2& center);

  public:
    RiggedShader rigged_shader;
    DebugShader debug_shader;

    void init();
    void render(SDL_Window* window, Player& player, BoxCollider& ground);

    inline glm::vec2 window_size() const { return window_size_; }

    bool draw_models = true;
    bool draw_bones = true;
    bool draw_wireframes = false;
    bool draw_splines = false;
};