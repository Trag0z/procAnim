#pragma once
#include "rendering/Renderer.h"
#include "Types.h"

class WeaponTrail {
    static const size_t NUM_SEGMENTS = 40;
    static const size_t NUM_VERTICES = NUM_SEGMENTS + 1;

    size_t next_weapon_position_index = 0;
    std::array<TrailShader::Vertex, NUM_VERTICES> weapon_positions;

    VertexArray<TrailShader::Vertex> vao;

  public:
    void init();
    void update(vec2 new_position);
    void render();
};