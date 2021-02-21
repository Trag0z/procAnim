#pragma once
#include "WeaponTrail.h"

void WeaponTrail::init() { // Set up weapon trail
    constexpr uint num_indices = WeaponTrail::SEGMENTS * 2;
    uint indices[num_indices];

    for (uint n_index = 1, n_trail = 0; n_index < num_indices;
         n_index += 2, ++n_trail) {
        indices[n_index - 1] = n_trail;
        indices[n_index] = n_trail + 1;
    }

    for (auto& pos : weapon_positions) {
        pos = vec2(0.0f);
    }

    vao.init(indices, num_indices, weapon_positions.data(),
             static_cast<GLuint>(weapon_positions.size()), GL_DYNAMIC_DRAW);
}

void WeaponTrail::update(vec2 new_position) {
    weapon_positions[next_array_pos++] = new_position;
    if (next_array_pos == weapon_positions.size())
        next_array_pos = 0;
}

void WeaponTrail::render() {
    vao.update_vertex_data(weapon_positions);
    vao.draw(GL_LINES);
}