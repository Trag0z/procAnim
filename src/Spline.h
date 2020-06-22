#pragma once
#include "pch.h"
#include "Input.h"
#include "VertexArray.h"
#include "Entity.h"
#include "Renderer.h"

struct Spline {
    static const size_t num_points = 4;
    static const size_t render_steps = 50;

    // P1, T1, T2, P2
    glm::vec2 points[num_points];
    glm::mat4 parameter_matrix;

    VertexArray<DebugShader::Vertex> line_vao;
    std::array<DebugShader::Vertex, render_steps> line_shader_vertices;

    VertexArray<DebugShader::Vertex> point_vao;
    std::array<DebugShader::Vertex, num_points> point_shader_vertices;

  public:
    // Points in format P1, T1, T2, P2
    void init(glm::vec2 points_[num_points] = nullptr);

    void update_render_data();
    glm::vec4 get_point_on_spline(float t) const;

    const glm::mat4 hermite_matrix = {2.0f,  -2.0f, 1.0f, 1.0f, -3.0f, 3.0f,
                                      -2.0f, -1.0f, 0.0f, 0.0f, 1.0f,  0.0f,
                                      1.0f,  0.0f,  0.0f, 0.0f};
};

class SplineEditor {
    const Entity* parent = nullptr;
    Spline* splines = nullptr;
    glm::vec2* spine_offsets = nullptr;
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
              std::string* names); // Only used for first setup of splines
    void update(const MouseKeyboardInput& input);
    void render(const Renderer& renderer, bool spline_edit_mode);
    void update_gui();
};