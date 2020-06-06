#pragma once
#include "pch.h"

constexpr size_t MAX_BONES_PER_VERTEX = 2;

// Not used until complex shaders are a thing
struct Vertex {
    glm::vec3 position;
    glm::vec2 uv_coord;
    GLuint bone_index[MAX_BONES_PER_VERTEX];
    float bone_weight[MAX_BONES_PER_VERTEX];
};

// Vertex format used by rigged shader
struct ShaderVertex {
    glm::vec4 pos;
    glm::vec2 uv_coord;
};

// Vertex format used by debug shader
struct DebugShaderVertex {
    glm::vec4 pos;
};

template <typename vertex_t> class VertexArrayData {
    GLuint vao_id, ebo_id, vbo_id;
    GLuint _num_indices, _num_vertices;

  public:
    void init(const GLuint* indices, GLuint num_indices,
              const vertex_t* vertices, GLuint num_vertices) {
        // Template specifications for acceptable vertex types are defined
        // below. If this function overload is called, something went wrong.
        printf("[ERROR] Trying to initialize VertexArrayData of an unknown "
               "vertex type");
        SDL_TriggerBreakpoint();
    }

    void update_vertex_data(std::vector<vertex_t> data) {
        SDL_assert(data.size() <= _num_vertices);
        glNamedBufferSubData(vbo_id, 0, sizeof(vertex_t) * data.size(),
                             data.data());
    }

    void update_vertex_data(vertex_t* data, GLuint num_vertices) {
        SDL_assert(num_vertices <= _num_vertices);
        glNamedBufferSubData(vbo_id, 0, sizeof(vertex_t) * num_vertices, data);
    }

    inline void draw(GLenum mode) const {
        glBindVertexArray(vao_id);
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