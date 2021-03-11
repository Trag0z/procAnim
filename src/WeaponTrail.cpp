#pragma once
#include "WeaponTrail.h"
#include "Util.h"
#include <glm/gtx/vector_angle.hpp>

void WeaponTrail::init(const float* max_angle_between_segments_,
                       const float* max_trail_length_) {
    max_angle_between_segments = max_angle_between_segments_;
    max_trail_length           = max_trail_length_;
    min_new_segment_length     = *max_trail_length / MAX_VERTICES;

    trail_length = 0.0f;

    // update() expects there to be at least two elements in weapon_positions,
    // so add dummys here to start out with
    weapon_positions.push_back({ vec2(0.0f), 0.0f });
    weapon_positions.push_back({ vec2(0.0f), 0.0f });

    vao.init(nullptr, MAX_VERTICES, GL_DYNAMIC_DRAW);
}

void WeaponTrail::update(vec2 new_position) {
    SDL_assert(weapon_positions.size() >= 2);

    if (weapon_positions.size() == 1) {
        trail_length = glm::length(new_position - weapon_positions.front().pos);

        SDL_assert(trail_length < *max_trail_length);
        weapon_positions.push_back(
          { new_position, trail_length / *max_trail_length });
        return;
    }

    auto iterator      = weapon_positions.rbegin();
    vec2 last_position = iterator->pos;
    ++iterator;
    vec2 last_segment = last_position - iterator->pos;
    vec2 new_segment  = new_position - last_position;

    float angle = glm::abs(glm::angle(new_segment, last_segment));
    if (angle > PI * 2.0f) { angle -= PI * 2.0f; }

    float new_segment_length = glm::length(new_segment);
    SDL_assert(new_segment_length < *max_trail_length);

    if (new_segment_length > 0.0f
        && new_segment_length < min_new_segment_length) {
        return;
    }

    SDL_assert(*max_angle_between_segments <= PI * 2.0f);
    if (new_segment_length == 0.0f || angle > *max_angle_between_segments) {
        weapon_positions.clear();
        weapon_positions.push_back({ last_position, 0.0f });
        trail_length = 0.0f;
    }

    trail_length += new_segment_length;
    weapon_positions.push_back(
      { new_position, trail_length / *max_trail_length });

    SDL_assert(*max_trail_length > 0.0f);
    while (trail_length > *max_trail_length) {
        // Remove positions until the length is short enough again
        auto it        = weapon_positions.begin();
        vec2 first_pos = it->pos;
        ++it;
        vec2 segment = first_pos - it->pos;
        weapon_positions.pop_front();

        trail_length -= glm::length(segment);
    }
}

void WeaponTrail::render() {
    std::vector<TrailShader::Vertex> ordered_vertices;
    ordered_vertices.reserve(weapon_positions.size());

    for (const auto& pos : weapon_positions) {
        ordered_vertices.push_back(pos);
    }

    vao.update_vertex_data(ordered_vertices);

    vao.draw(GL_LINE_STRIP, static_cast<GLuint>(ordered_vertices.size()));
}