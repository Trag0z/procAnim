#pragma once
#include "pch.h"
#include "Input.h"
#include "VertexArray.h"
#include "Entity.h"

struct Bone;
class Renderer;
struct Animation;

struct Spline {
    static const size_t NUM_POINTS = 4;
    static const size_t RENDER_STEPS = 50;

    enum { P1 = 0, T1 = 1, T2 = 2, P2 = 3 };

    glm::vec2 points[NUM_POINTS];
    glm::mat4 parameter_matrix;

    VertexArray<DebugShader::Vertex> line_vao;
    std::array<DebugShader::Vertex, RENDER_STEPS> line_shader_vertices;

    VertexArray<DebugShader::Vertex> point_vao;
    std::array<DebugShader::Vertex, NUM_POINTS> point_shader_vertices;

    bool vertices_initialized = false;

  public:
    // Points in format P1, T1, T2, P2
    void init(glm::vec2 points_[NUM_POINTS] = nullptr);

    void update_render_data();
    glm::vec2 get_point_on_spline(float t) const;

    const glm::mat4 hermite_matrix = {2.0f,  -2.0f, 1.0f, 1.0f, -3.0f, 3.0f,
                                      -2.0f, -1.0f, 0.0f, 0.0f, 1.0f,  0.0f,
                                      1.0f,  0.0f,  0.0f, 0.0f};
};

class SplineEditor {
    const Entity* parent = nullptr;

    Animation* animations = nullptr;
    size_t num_animations;

    // @CLEANUP: Does it ever use the second bone?
    const Bone* limb_bones[4][2];

    size_t selected_animation_index = static_cast<size_t>(-1);
    size_t selected_spline_index = static_cast<size_t>(-1);
    size_t selected_point_index = static_cast<size_t>(-1);

    bool creating_new_spline = false;
    bool first_point_set = false;

    bool connect_point_pairs = true;
    bool connect_tangent_pairs = false;
    bool mirror_to_opposing_limb = false;

    static const size_t MAX_SAVE_PATH_LENGTH = 128;
    char* save_path = nullptr;

    static const size_t CIRCLE_SEGMENTS = 30;
    VertexArray<DebugShader::Vertex> circle_vao;

    void save_splines(bool get_new_file_path = false);
    void load_splines(const char* path = nullptr);

    void set_spline_point(glm::vec2 p,
                          size_t point_index = static_cast<size_t>(-1),
                          size_t spline_index = static_cast<size_t>(-1));

    bool update_gui();

  public:
    void init(const Entity* parent_, Animation* animations_,
              const Bone* limb_bones_[4][2], const char* spline_path);
    // Only used for first setup of splines
    void init(const Entity* parent_, Spline* splines_, size_t num_splines_,
              const std::string* spline_names_, const Bone* limb_bones_[4][2]);
    bool update(const MouseKeyboardInput& input);
    void render(const Renderer& renderer, bool spline_edit_mode);
};