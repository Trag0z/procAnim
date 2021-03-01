#pragma once
#include "rendering/Renderer.h"
#include "Types.h"
#include <list>

class WeaponTrail {
    static const size_t MAX_VERTICES = 400;

    std::list<TrailShader::Vertex> weapon_positions;
    VertexArray<TrailShader::Vertex> vao;

    const float* max_angle_between_segments;
    const float* max_trail_length;
    float min_new_segment_length;

  public:
    float trail_length;

    void init(const float* max_angle_between_segments_,
              const float* max_trail_length_);
    void update(vec2 new_position);
    void render();
};