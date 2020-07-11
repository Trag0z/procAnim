#pragma once
#include "pch.h"

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
              const vertex_t* vertices, GLuint num_vertices,
              GLenum usage = GL_DYNAMIC_DRAW) {
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
