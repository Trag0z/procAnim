#pragma once
#include "rendering/Renderer.h"
#include "Types.h"

class WeaponTrail {
    static const size_t SEGMENTS = 40;
    VertexArray<DebugShader::Vertex> vao;
    size_t next_array_pos = 0;
    std::array<vec2, SEGMENTS + 1> weapon_positions;

  public:
    void init();
    void update(vec2 new_position);
    void render();
};