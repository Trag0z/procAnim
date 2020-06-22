#pragma once
#include "pch.h"
#include "Renderer.h"

void Renderer::init() {
    rigged_shader = RiggedShader("../src/shaders/rigged.vert",
                                 "../src/shaders/rigged.frag");
    rigged_shader.set_projection(&projection);

    debug_shader =
        DebugShader("../src/shaders/debug.vert", "../src/shaders/debug.frag");
    debug_shader.set_projection(&projection);
}