#pragma once
#include "pch.h"
#include "Shaders.h"
#include "Texture.h"

class Player;
struct BoxCollider;
class Background;

class Renderer {
    glm::vec2 window_size_ = {1920.0f, 1080.0f};
    glm::vec2 camera_center = {0.0f, 0.0f};

    void update_camera(const glm::vec2& center);

  public:
    RiggedShader rigged_shader;
    TexturedShader textured_shader;
    DebugShader debug_shader;

    void init();
    void render(SDL_Window* window, Background& background, Player& player,
                BoxCollider& ground);

    glm::vec2 window_size() const noexcept;
    glm::vec2 camera_position() const noexcept;

    bool draw_models = true;
    bool draw_bones = true;
    bool draw_wireframes = false;
    bool draw_splines = false;
};