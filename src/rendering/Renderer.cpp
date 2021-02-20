#pragma once
#include "Renderer.h"
#include "../Player.h"
#include "../Background.h"
#include "../Level.h"
#include <glm/gtx/matrix_transform_2d.hpp>

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

void Renderer::update(float delta_time) {
    vec2 screen_shake_offset = vec2(0.0f);
    if (screen_shake.duration > 0.0f) {
        screen_shake_offset.x = screen_shake.intensity *
                                screen_shake.noise[0].normalizedOctaveNoise1D(
                                    screen_shake.noise_pos, 6);
        screen_shake_offset.y = screen_shake.intensity *
                                screen_shake.noise[1].normalizedOctaveNoise1D(
                                    screen_shake.noise_pos, 6);

        screen_shake.noise_pos += delta_time * screen_shake.speed;
        screen_shake.duration -= delta_time;
    }

    // Update the camera
    // NOTE: We don't use glm::ortho or something similar here. Is that correct?
    glm::mat3 cam = glm::scale(glm::mat3(1.0f),
                               glm::vec2(2.0f / window_size_.x * zoom_factor_,
                                         2.0f / window_size_.y * zoom_factor_));
    cam = glm::translate(cam, -camera_center_ + screen_shake_offset);

    debug_shader.set_camera(&cam);
    textured_shader.set_camera(&cam);
    rigged_shader.set_camera(&cam);
    rigged_debug_shader.set_camera(&cam);
    bone_shader.set_camera(&cam);
}

void Renderer::shake_screen(float intensity, float duration, float speed) {
    screen_shake.intensity = intensity;
    screen_shake.duration = duration;
    screen_shake.speed = speed;
}

glm::vec2 Renderer::window_size() const noexcept { return window_size_; }

glm::vec2 Renderer::camera_position() const noexcept {
    return camera_center_ - window_size_ * 0.5f / zoom_factor_;
}

glm::vec2 Renderer::camera_center() const noexcept { return camera_center_; }

float Renderer::zoom_factor() const noexcept { return zoom_factor_; }

// NOTE: The screen space origin is on the top left!
glm::vec2 Renderer::screen_to_world_space(glm::vec2 screen_pos) const noexcept {
    glm::vec2 result =
        camera_position() +
        glm::vec2(screen_pos.x / zoom_factor_,
                  (window_size_.y - screen_pos.y) / zoom_factor_);
    return result;
}