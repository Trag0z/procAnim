#pragma once
#include "pch.h"
#include "Shaders.h"

constexpr size_t MAX_BONES_PER_VERTEX = 2;

struct RiggedVertex {
    glm::vec2 position;
    glm::vec2 uv_coord;
    GLuint bone_index[MAX_BONES_PER_VERTEX];
    float bone_weight[MAX_BONES_PER_VERTEX];
};

template <typename vertex_t> class VertexArray {
    GLuint vao_id, ebo_id, vbo_id;
    GLuint num_indices_, num_vertices_;

  public:
    void init(const GLuint* indices, GLuint num_indices,
              const vertex_t* vertices, GLuint num_vertices) {
        // Template specifications for acceptable vertex types are defined
        // below. If this function overload is called, something went wrong.
        printf("[ERROR] Trying to initialize VertexArray of an unknown "
               "vertex type");
        SDL_TriggerBreakpoint();
    }

    void update_vertex_data(std::vector<vertex_t> data) {
        SDL_assert(data.size() <= num_vertices_);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_id);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertex_t) * data.size(),
                        data.data());
        // glNamedBufferSubData(vbo_id, 0, sizeof(vertex_t) * data.size(),
        //                      data.data());
    }

    void update_vertex_data(vertex_t* data, GLuint num_vertices) {
        SDL_assert(num_vertices <= num_vertices_);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_id);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertex_t) * num_vertices,
                        data);
        // glNamedBufferSubData(vbo_id, 0, sizeof(vertex_t) * num_vertices,
        // data);
    }

    inline void draw(GLenum mode) const {
        glBindVertexArray(vao_id);
        glDrawElements(mode, num_indices_, GL_UNSIGNED_INT, 0);
    }
};

void VertexArray<RiggedShader::Vertex>::init(
    const GLuint* indices, GLuint num_indices,
    const RiggedShader::Vertex* vertices, GLuint num_vertices) {
    num_indices_ = num_indices;
    num_vertices_ = num_vertices;

    // Upload data to GPU
    glGenVertexArrays(1, &vao_id);
    glBindVertexArray(vao_id);

    // Create index buffer
    glGenBuffers(1, &ebo_id);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_id);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * num_indices_,
                 indices, GL_STATIC_DRAW);

    // Create vertex buffer
    glGenBuffers(1, &vbo_id);

    glBindBuffer(GL_ARRAY_BUFFER, vbo_id);
    glBufferData(GL_ARRAY_BUFFER, sizeof(RiggedShader::Vertex) * num_vertices_,
                 vertices, GL_DYNAMIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE,
                          sizeof(RiggedShader::Vertex),
                          reinterpret_cast<void*>(0));
    glEnableVertexAttribArray(0);
    // uvCoord attribute
    glVertexAttribPointer(
        1, 2, GL_FLOAT, GL_FALSE, sizeof(RiggedShader::Vertex),
        reinterpret_cast<void*>(offsetof(RiggedShader::Vertex, uv_coord)));
    glEnableVertexAttribArray(1);

    // Reset vertex array binding for error safety
    glBindVertexArray(0);
}

void VertexArray<DebugShader::Vertex>::init(const GLuint* indices,
                                            GLuint num_indices,
                                            const DebugShader::Vertex* vertices,
                                            GLuint num_vertices) {
    num_indices_ = num_indices;
    num_vertices_ = num_vertices;

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
    glBufferData(GL_ARRAY_BUFFER, sizeof(DebugShader::Vertex) * num_vertices,
                 vertices, GL_DYNAMIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(DebugShader::Vertex),
                          reinterpret_cast<void*>(0));
    glEnableVertexAttribArray(0);

    // Reset vertex array binding for error safety
    glBindVertexArray(0);
}