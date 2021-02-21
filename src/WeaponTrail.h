#pragma once
#include "rendering/Renderer.h"
#include "Types.h"

class WeaponTrail {
    static const size_t SEGMENTS = 40;
    static const uint num_indices = WeaponTrail::SEGMENTS * 2;
    static uint indices[num_indices];
    static bool statics_initialized;

    VertexArray<TrailShader::Vertex> vao;
    size_t next_weapon_position_index = 0;
    std::array<TrailShader::Vertex, SEGMENTS + 1> weapon_positions;

  public:
    void init();
    void update(vec2 new_position);
    void render();
};