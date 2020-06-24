#pragma once
#include "pch.h"
#include "Input.h"
#include "VertexArray.h"
#include "Entity.h"
#include "Renderer.h"

struct Bone;

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
    size_t num_splines;

    glm::vec2* selected_point = nullptr;
    size_t selected_spline_index;

    bool creating_new_spline = false;
    bool first_point_set = false;

    static const size_t MAX_SPLINE_NAME_LENGTH = 32;
    std::vector<std::string> spline_names;

    static const size_t MAX_SAVE_PATH_LENGTH = 128;
    char* save_path = nullptr;

    const Bone* limb_bones[4][2];
    static const size_t CIRCLE_SEGMENTS = 30;
    VertexArray<DebugShader::Vertex> circle_vao;

    void save_splines(bool get_new_file_path = false);
    void load_splines();

  public:
    void init(const Entity* parent_, Spline* splines_,
              const Bone* limb_bones_[4][2], const char* spline_path);
    // Only used for first setup of splines
    void init(const Entity* parent_, Spline* splines_, size_t num_splines_,
              const std::string* spline_names_, const Bone* limb_bones_[4][2]);
    void update(const MouseKeyboardInput& input);
    void render(const Renderer& renderer, bool spline_edit_mode);
    void update_gui();
};