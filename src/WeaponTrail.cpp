#pragma once
#include "WeaponTrail.h"

bool WeaponTrail::statics_initialized = false;
uint WeaponTrail::indices[WeaponTrail::num_indices];

void WeaponTrail::init() {
    if (!statics_initialized) {
        for (uint n_index = 1, n_trail = 0; n_index < num_indices;
             n_index += 2, ++n_trail) {
            indices[n_index - 1] = n_trail;
            indices[n_index] = n_trail + 1;
        }
        statics_initialized = true;
    }

    for (auto& pos : weapon_positions) {
        pos.pos = vec2(0.0f);
        pos.age = 1.0f;
    }

    vao.init(indices, num_indices, weapon_positions.data(),
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
    vao.update_vertex_data(weapon_positions);

    uint oldest_elem_index = static_cast<uint>(next_weapon_position_index) * 2;
    if (oldest_elem_index >= num_indices) {
        oldest_elem_index = 0;
    }

    vao.draw(GL_LINES, num_indices - oldest_elem_index,
             &indices[oldest_elem_index]);

    uint remaining_indices = num_indices - (num_indices - oldest_elem_index);
    vao.draw(GL_LINES, remaining_indices, indices);
}