#pragma once
#include "rendering/Renderer.h"
#include "Types.h"
#include <list>

class WeaponTrail {
    static const size_t NUM_SEGMENTS = 30;
    static const size_t NUM_VERTICES = NUM_SEGMENTS + 1;

    std::list<TrailShader::Vertex> weapon_positions;

    VertexArray<TrailShader::Vertex> vao;

  public:
    void init();
    void update(vec2 new_position);
    void render();

    float continuous_trail_length(float max_angle_between_segments);
};