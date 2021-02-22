#pragma once
#include "WeaponTrail.h"
#include <glm/gtx/vector_angle.hpp>

void WeaponTrail::init() {
    uint indices[NUM_VERTICES];
    for (uint n_index = 0; n_index < NUM_VERTICES; ++n_index) {
        indices[n_index] = n_index;
        weapon_positions.push_back({vec2(0.0f), 1.0f});
    }

    vao.init(indices, NUM_VERTICES, nullptr, NUM_VERTICES, GL_DYNAMIC_DRAW);
}

void WeaponTrail::update(vec2 new_position) {
    // Let the old positions age
    const float aging_per_update =
        1.0f / static_cast<float>(weapon_positions.size());

    for (auto& pos : weapon_positions) {
        pos.age += aging_per_update;
    };

    // Add the new value
    weapon_positions.push_back({new_position, 0.0f});
    weapon_positions.pop_front();
}

void WeaponTrail::render() {
    std::vector<TrailShader::Vertex> ordered_vertices;
    ordered_vertices.reserve(NUM_VERTICES);

    for (const auto& pos : weapon_positions) {
        ordered_vertices.push_back(pos);
    }

    vao.update_vertex_data(ordered_vertices);

    vao.draw(GL_LINE_STRIP);
}

float WeaponTrail::continuous_trail_length(float max_angle_between_segments) {
    float result = 0.0f;
    for (auto it = weapon_positions.rend(); it != weapon_positions.rbegin();
         ++it) {
        auto it2 = it;

        vec2 seg1 = it2->pos - (--it2)->pos;
        vec2 seg2 = it2->pos - (--it2)->pos;

        result += glm::length(seg1);

        if (glm::angle(seg1, seg2) > max_angle_between_segments) {
            return result;
        }
    }
    return result;
}