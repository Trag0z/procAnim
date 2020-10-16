#pragma once
#include "pch.h"

template <typename vertex_t> class VertexArray {
    GLuint vao_id, ebo_id, vbo_id;
    GLuint num_indices_, num_vertices_;
    GLenum usage_;

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

    void update_vertex_data(const std::vector<vertex_t> data) {
        SDL_assert(usage_ == GL_DYNAMIC_DRAW && data.size() <= num_vertices_);

        glBindBuffer(GL_ARRAY_BUFFER, vbo_id);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertex_t) * data.size(),
                        data.data());
    }

    void update_vertex_data(const vertex_t* data, GLuint num_vertices) {
        SDL_assert(usage_ == GL_DYNAMIC_DRAW && num_vertices <= num_vertices_);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_id);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertex_t) * num_vertices,
                        data);
    }

    void draw(GLenum mode) const {
        glBindVertexArray(vao_id);
        glDrawElements(mode, num_indices_, GL_UNSIGNED_INT, 0);
    }
};
