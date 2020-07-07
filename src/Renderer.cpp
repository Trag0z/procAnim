#pragma once
#include "pch.h"
#include "Renderer.h"
#include "Player.h"
#include "Background.h"

void Renderer::update_camera(const glm::vec2& center) {
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

void Renderer::render(SDL_Window* window, Background& background,
                      Player& player, BoxCollider& ground) {
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    camera_center = player.position() + glm::vec2(0.0f, 200.0f);

    update_camera(camera_center);

    background.render(*this, camera_center);

    ground.render(*this);

    player.render(*this);

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    SDL_GL_SwapWindow(window);
}

glm::vec2 Renderer::window_size() const noexcept { return window_size_; }

glm::vec2 Renderer::camera_position() const noexcept {
    return camera_center - window_size_ * 0.5f;
}