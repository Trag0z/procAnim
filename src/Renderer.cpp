#pragma once
#include "pch.h"
#include "Renderer.h"

// glm::mat3 calculate_camera(float left, float right, float top,
//                                   float bottom) {
//     glm::mat3 ret =
//         glm::translate(glm::mat3(1.0f),
//                        glm::vec2((left + right) * 0.5f, (top + bottom) *
//                        0.5f));
//     ret = glm::scale(ret,
//                      glm::vec2(2.0f / (left + right), 2.0f / (top +
//                      bottom)));
//     return ret;
// }

void Renderer::init() {
    rigged_shader = RiggedShader("../src/shaders/rigged.vert",
                                 "../src/shaders/rigged.frag");
    rigged_shader.set_camera(&camera);

    debug_shader =
        DebugShader("../src/shaders/debug.vert", "../src/shaders/debug.frag");
    debug_shader.set_camera(&camera);
}