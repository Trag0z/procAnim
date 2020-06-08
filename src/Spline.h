#pragma once
#include "pch.h"
#include "Input.h"
#include "VertexArrayData.h"
#include "Entity.h"

struct RenderData;

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
    void init(glm::vec2 points_[num_points] = nullptr);

    void update_render_data();

    glm::mat4 hermite_matrix = {2.0f,  -2.0f, 1.0f, 1.0f, -3.0f, 3.0f,
                                -2.0f, -1.0f, 0.0f, 0.0f, 1.0f,  0.0f,
                                1.0f,  0.0f,  0.0f, 0.0f};
};

class SplineEditor {
    const Entity* parent = nullptr;
    Spline* splines = nullptr;
    size_t num_splines;
    std::vector<std::string> spline_names;
    const char* save_path;

    glm::vec2* selected_point = nullptr;
    size_t selected_spline_index;

    bool creating_new_spline = false;
    bool first_point_set = false;

    static const size_t max_spline_name_length;

    void save_splines();

  public:
    void init(const Entity* parent, Spline* splines_, const char* save_path_);
    void init(const Entity* parent, Spline* splines_, size_t num_splines_,
              std::string* names);
    void update(const MouseKeyboardInput& input);
    void render(const RenderData& render_data);
    void update_gui();
};