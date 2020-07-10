#pragma once
#include "pch.h"
#include "Renderer.h"
#include "Player.h"
#include "Background.h"
#include "level/Level.h"

void Renderer::update_camera(const glm::vec2& center) {
    camera_center_ = center;

    glm::mat3 cam =
        glm::scale(glm::mat3(1.0f),
                   glm::vec2(2.0f / window_size_.x, 2.0f / window_size_.y));
    cam = glm::translate(cam, -center);

    rigged_shader.set_camera(&cam);
    textured_shader.set_camera(&cam);
    debug_shader.set_camera(&cam);
}

void Renderer::init() {
    rigged_shader = RiggedShader("../src/shaders/rigged.vert",
                                 "../src/shaders/rigged.frag");

    textured_shader = TexturedShader("../src/shaders/textured.vert",
                                     "../src/shaders/textured.frag");

    debug_shader =
        DebugShader("../src/shaders/debug.vert", "../src/shaders/debug.frag");
}

glm::vec2 Renderer::window_size() const noexcept { return window_size_; }

glm::vec2 Renderer::camera_position() const noexcept {
    return camera_center_ - window_size_ * 0.5f;
}

glm::vec2 Renderer::camera_center() const noexcept { return camera_center_; }