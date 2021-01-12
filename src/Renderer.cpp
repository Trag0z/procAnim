#pragma once
#include "pch.h"
#include "Renderer.h"
#include "Player.h"
#include "Background.h"
#include "Level.h"

void Renderer::update_camera(const glm::vec2& center, float zoom_factor) {
    camera_center_ = center;

    if (zoom_factor != 0.0f)
        zoom_factor_ = zoom_factor;

    glm::mat3 cam = glm::scale(glm::mat3(1.0f),
                               glm::vec2(2.0f / window_size_.x * zoom_factor_,
                                         2.0f / window_size_.y * zoom_factor_));
    cam = glm::translate(cam, -center);

    debug_shader.set_camera(&cam);
    textured_shader.set_camera(&cam);
    rigged_shader.set_camera(&cam);
    rigged_debug_shader.set_camera(&cam);
    bone_shader.set_camera(&cam);
}

void Renderer::init() {
    debug_shader =
        DebugShader("../src/shaders/debug.vert", "../src/shaders/debug.frag");

    textured_shader = TexturedShader("../src/shaders/textured.vert",
                                     "../src/shaders/textured.frag");

    rigged_shader = RiggedShader("../src/shaders/rigged.vert",
                                 "../src/shaders/rigged.frag");

    rigged_debug_shader = RiggedDebugShader("../src/shaders/rigged_debug.vert",
                                            "../src/shaders/rigged_debug.frag");

    bone_shader =
        BoneShader("../src/shaders/bone.vert", "../src/shaders/bone.frag");
}

glm::vec2 Renderer::window_size() const noexcept { return window_size_; }

glm::vec2 Renderer::camera_position() const noexcept {
    return camera_center_ - window_size_ * 0.5f;
}

glm::vec2 Renderer::camera_center() const noexcept { return camera_center_; }

float Renderer::zoom_factor() const noexcept { return zoom_factor_; }