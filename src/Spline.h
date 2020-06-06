#pragma once
#include "pch.h"
#include "Input.h"
#include "VertexArrayData.h"

// Simple Cardinal spline
struct Spline {
    static const size_t num_points = 4;
    static const size_t render_steps = 50;

    // P1, P2, T1, T2
    glm::vec2 points[num_points];

    VertexArrayData<DebugShaderVertex> line_vao;
    std::array<DebugShaderVertex, render_steps> line_shader_vertices;

    VertexArrayData<DebugShaderVertex> point_vao;
    std::array<DebugShaderVertex, num_points> point_shader_vertices;

  public:
    // Points in format P1, T1, T2, P2
    Spline(glm::vec2 points_[num_points]);

    void update_render_data();

    glm::mat4 hermite_matrix = {2.0f,  -2.0f, 1.0f, 1.0f, -3.0f, 3.0f,
                                -2.0f, -1.0f, 0.0f, 0.0f, 1.0f,  0.0f,
                                1.0f,  0.0f,  0.0f, 0.0f};
};

class SplineEditor {
    std::vector<Spline> splines;

    glm::vec2* selected_point = nullptr;
    size_t selected_spline_index;

    bool creating_new_spline = false;
    bool first_point_set = false;

  public:
    void update(const MouseKeyboardInput& input);
    void update_gui();
    void render(GLuint shader_id, GLuint color_uniform_loc);
};