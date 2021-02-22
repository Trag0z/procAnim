#pragma once
#include "WeaponTrail.h"

void WeaponTrail::init() {
    uint indices[NUM_VERTICES];
    for (uint n_index = 0; n_index < NUM_VERTICES; ++n_index) {
        indices[n_index] = n_index;
    }

    for (auto& pos : weapon_positions) {
        pos.pos = vec2(0.0f);
        pos.age = 1.0f;
    }

    vao.init(indices, NUM_VERTICES, weapon_positions.data(),
             static_cast<GLuint>(weapon_positions.size()), GL_DYNAMIC_DRAW);
}

void WeaponTrail::update(vec2 new_position) {
    // Let the old positions age
    const float aging_per_update =
        1.0f / static_cast<float>(weapon_positions.size());

    for (auto& pos : weapon_positions) {
        pos.age += aging_per_update;
    };

    // Add the new value
    weapon_positions[next_weapon_position_index++] = {new_position, 0.0f};
    if (next_weapon_position_index == weapon_positions.size())
        next_weapon_position_index = 0;
}

void WeaponTrail::render() {
    std::array<TrailShader::Vertex, NUM_VERTICES> ordered_vertices;
    ordered_vertices[0] = weapon_positions[0];
    size_t ordered_vertices_index = 0;

    for (size_t i = next_weapon_position_index; i < NUM_VERTICES; ++i) {
        ordered_vertices[ordered_vertices_index++] = weapon_positions[i];
    }

    for (size_t i = 0; ordered_vertices_index < NUM_VERTICES; ++i) {
        ordered_vertices[ordered_vertices_index++] = weapon_positions[i];
    }
    // #pragma warning(disable : 4701)
    vao.update_vertex_data(ordered_vertices);

    vao.draw(GL_LINE_STRIP);

    uint indices[2] = {NUM_VERTICES - 5, NUM_VERTICES - 1};

    vao.draw(GL_POINTS, 2, indices);
}