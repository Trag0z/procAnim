#pragma once
#include <array>
#include "Input.h"
#include "rendering/Shaders.h"
#include "Entity.h"

class Renderer;
class SplineEditor;
struct SplineSet;
struct Limb;

enum SplinePointName { P1 = 0, T1 = 1, T2 = 2, P2 = 3 };

class Spline {
  public:
    static const size_t NUM_POINTS = 4;

    // Points in format P1, T1, T2, P2.
    // The values of T1 and T2 are the coordinates relative to P1 and P2,
    // respectively.
    void init(const glm::vec2 points[NUM_POINTS] = nullptr);
    void render(const Renderer& renderer, bool draw_points = false) const;

    const glm::vec2& point(SplinePointName p) const;
    const glm::vec2* points() const;

    // If P1 or P2 are set, T1 or T2 are moved with them.
    void set_point(SplinePointName name, glm::vec2 point);

    void set_points(const glm::vec2 new_points[NUM_POINTS]);
    void update_render_data();
    glm::vec2 get_point_on_spline(float t) const;

    static const glm::mat4 HERMITE_MATRIX;

  private:
    static const size_t RENDER_STEPS = 50;

    // T1 and T2 are relative to P1/P2.
    glm::vec2 points_[NUM_POINTS];
    glm::mat4 parameter_matrix;

    VertexArray<DebugShader::Vertex> line_vao;
    std::array<DebugShader::Vertex, RENDER_STEPS> line_shader_vertices;

    VertexArray<DebugShader::Vertex> point_vao;
    std::array<DebugShader::Vertex, 2> point_shader_vertices;

    bool vertices_initialized = false;

    // SplineEditor has to access the points and VertexArrays in order to
    // dislpay and modify them.
    friend SplineEditor;
};

class SplineEditor {
  public:
    void init(const Entity* parent_,
              SplineSet* splines_,
              Limb* limbs_,
              const char* spline_path);
    bool update(const MouseKeyboardInput& input);
    void render(const Renderer& renderer, bool spline_edit_mode);

  private:
    const Entity* parent;

    static const size_t NUM_ANIMATIONS            = 3;
    static const size_t NUM_SPLINES_PER_ANIMATION = 2;

    SplineSet* spline_set;
    Limb* limbs;

    enum SelectedAnimation {
        WALK = 0,
        RUN  = 1,
        IDLE = 2,
        NONE = 3
    } selected_animation;

    size_t selected_spline_index = static_cast<size_t>(-1);
    size_t selected_point_index  = static_cast<size_t>(-1);

    bool creating_new_spline = false;
    bool first_point_set     = false;

    bool connect_point_pairs   = true;
    bool connect_tangent_pairs = false;

    std::string save_path;

    static const size_t CIRCLE_SEGMENTS = 30;
    VertexArray<DebugShader::Vertex> circle_vao;

    VertexArray<DebugShader::Vertex> tangents_vao;

    void save_splines(bool get_new_file_path = false);
    void load_splines(const std::string& path);

    // Sets the point at index_point_index of the spline at spline_index to
    // position p. If absolute_tangent_pos is set to true, the function assumes
    // that p is given in absolute (model) coordinates and adjusts it to be
    // relative to either P1 or P2. If absolute_tangent_pos is false, it just
    // uses the value of p.
    void set_spline_point(glm::vec2 p,
                          size_t point_index        = static_cast<size_t>(-1),
                          size_t spline_index       = static_cast<size_t>(-1),
                          bool absolute_tangent_pos = true);

    bool update_gui();
};