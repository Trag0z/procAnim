#pragma once
#include "pch.h"
#include "Types.h"
#include "VertexArrayData.h"
#include "Animator.h" // Cyclic dependency?

struct LimbAnimator;

namespace MeshDetail {

constexpr size_t MAX_BONES_PER_VERTEX = 2;

struct Vertex {
    glm::vec3 position;
    glm::vec2 uv_coord;
    GLuint bone_index[MAX_BONES_PER_VERTEX];
    float bone_weight[MAX_BONES_PER_VERTEX];
};

struct ShaderVertex {
    glm::vec4 pos;
    glm::vec2 uv_coord;
};

struct DebugShaderVertex {
    glm::vec4 pos;
};

} // namespace MeshDetail

using namespace MeshDetail;

template <typename vertex_t> class VertexArrayData {
    GLuint vao_id, ebo_id, vbo_id;
    GLuint _num_indices, _num_vertices;

  public:
    void init(const GLuint* indices, GLuint num_indices,
              const vertex_t* vertices, GLuint num_vertices) {
        printf("[ERROR] You are trying to initialize VertexArrayData of an "
               "unknown vertex type");
        SDL_assert(false);
    }

    void update_vertex_data(std::vector<vertex_t> data) {
        SDL_assert(data.size() <= _num_vertices);
        glNamedBufferSubData(vbo_id, 0, sizeof(vertex_t) * data.size(),
                             data.data());
    }

    void update_vertex_data(GLuint num_vertices, vertex_t* data) {
        SDL_assert(num_vertices <= _num_vertices);
        glNamedBufferSubData(vbo_id, 0, sizeof(vertex_t) * num_vertices, data);
    }

    void bind() { glBindVertexArray(vao_id); }

    void draw(GLenum mode) {
        glDrawElements(mode, _num_indices, GL_UNSIGNED_INT, 0);
    }
};

void VertexArrayData<ShaderVertex>::init(const GLuint* indices,
                                         GLuint num_indices,
                                         const ShaderVertex* vertices,
                                         GLuint num_vertices) {
    _num_indices = num_indices;
    _num_vertices = num_vertices;

    // Upload data to GPU
    glGenVertexArrays(1, &vao_id);
    glBindVertexArray(vao_id);

    // Create index buffer
    glGenBuffers(1, &ebo_id);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_id);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * _num_indices,
                 indices, GL_STATIC_DRAW);

    // Create vertex buffer
    glGenBuffers(1, &vbo_id);

    glBindBuffer(GL_ARRAY_BUFFER, vbo_id);
    glBufferData(GL_ARRAY_BUFFER, sizeof(ShaderVertex) * _num_vertices,
                 vertices, GL_DYNAMIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(ShaderVertex),
                          reinterpret_cast<void*>(0));
    glEnableVertexAttribArray(0);
    // uvCoord attribute
    glVertexAttribPointer(
        1, 2, GL_FLOAT, GL_FALSE, sizeof(ShaderVertex),
        reinterpret_cast<void*>(offsetof(ShaderVertex, uv_coord)));
    glEnableVertexAttribArray(1);

    // position attribute
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(ShaderVertex),
                          reinterpret_cast<void*>(0));
    glEnableVertexAttribArray(0);
    // uvCoord attribute
    glVertexAttribPointer(
        1, 2, GL_FLOAT, GL_FALSE, sizeof(ShaderVertex),
        reinterpret_cast<void*>(offsetof(ShaderVertex, uv_coord)));
    glEnableVertexAttribArray(1);

    // Reset vertex array binding for error safety
    glBindVertexArray(0);
}

void VertexArrayData<DebugShaderVertex>::init(const GLuint* indices,
                                              GLuint num_indices,
                                              const DebugShaderVertex* vertices,
                                              GLuint num_vertices) {
    _num_indices = num_indices;
    _num_vertices = num_vertices;

    // Upload data to GPU
    glGenVertexArrays(1, &vao_id);
    glBindVertexArray(vao_id);

    // Create index buffer
    glGenBuffers(1, &ebo_id);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_id);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * num_indices, indices,
                 GL_STATIC_DRAW);

    // Create vertex buffer
    glGenBuffers(1, &vbo_id);

    glBindBuffer(GL_ARRAY_BUFFER, vbo_id);
    glBufferData(GL_ARRAY_BUFFER, sizeof(DebugShaderVertex) * num_vertices,
                 vertices, GL_DYNAMIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(DebugShaderVertex),
                          reinterpret_cast<void*>(0));
    glEnableVertexAttribArray(0);

    // Reset vertex array binding for error safety
    glBindVertexArray(0);
}

struct Bone;

struct LimbAnimator {
    Bone* bone1;
    Bone* bone2;

    glm::vec4 target_world_pos;

    VertexArrayData<DebugShaderVertex> vao;

    const static float animation_speed;

    LimbAnimator(Bone* b1, Bone* b2) : bone1(b1), bone2(b2) {
        target_world_pos = glm::vec4{0.0f, 0.0f, 0.0f, 0.0f};

        // Init render data for rendering target_world_pos as a point
        GLuint index = 0;

        vao.init(&index, 1, NULL, 1);
    }

    void update();
};

struct Bone {
    std::string name;
    size_t parent;
    glm::mat4 inverse_bind_pose_transform;
    glm::mat4 bone_transform;
    glm::vec4 head, tail;  // Positions in world coordinates
    float rotation = 0.0f; // Degrees around z-Axis
    float length;

    static constexpr size_t INDEX_NOT_FOUND = UINT_MAX;
    inline bool has_parent() const { return parent != INDEX_NOT_FOUND; }
};

struct RiggedMesh {
    std::vector<Vertex> vertices;

    VertexArrayData<ShaderVertex> vao;
    std::vector<ShaderVertex> shader_vertices;

    VertexArrayData<DebugShaderVertex> bones_vao;
    std::vector<DebugShaderVertex> bones_shader_vertices;

    std::vector<Bone> bones;

    std::vector<LimbAnimator> animators;

    size_t find_bone_index(const char* name) const;
    void load_from_file(const char* file);
};
