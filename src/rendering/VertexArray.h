#pragma once
#include <vector>
#include <sdl/SDL_assert.h>

// Manages an array of vertices of type vertex_t in GPU memory.
template<typename vertex_t>
class VertexArray {
    GLuint vao_id, ebo_id, vbo_id;
    GLuint num_indices_, num_vertices_;
    GLenum usage_;

#ifdef SHADER_DEBUG
    std::vector<vertex_t> vertex_data;
#endif

  public:
    void init(const GLuint* indices,
              GLuint num_indices,
              const vertex_t* vertices,
              GLuint num_vertices,
              GLenum usage) {
        // Template specifications for acceptable vertex types are defined
        // in Shaders.h. If this function overload is called, something went
        // wrong.
        printf(
          "[ERROR] Trying to initialize VertexArray of an unknown "
          "vertex type!\n");
        SDL_TriggerBreakpoint();
    }

    void init(const vertex_t* vertices, GLuint num_vertices, GLenum usage) {
        // Template specifications for acceptable vertex types are defined
        // in Shaders.h. If this function overload is called, something went
        // wrong.
        printf(
          "[ERROR] Trying to initialize VertexArray of an unknown "
          "vertex type!\n");
        SDL_TriggerBreakpoint();
    }

    void update_vertex_data(const std::vector<vertex_t> data) {
        SDL_assert(usage_ == GL_DYNAMIC_DRAW && data.size() <= num_vertices_);
#ifdef SHADER_DEBUG
        vertex_data = data;
#endif

        glBindBuffer(GL_ARRAY_BUFFER, vbo_id);
        glBufferSubData(
          GL_ARRAY_BUFFER, 0, sizeof(vertex_t) * data.size(), data.data());
    }

    template<size_t array_size>
    void update_vertex_data(const std::array<vertex_t, array_size> data) {
        SDL_assert(usage_ == GL_DYNAMIC_DRAW && data.size() <= num_vertices_);
#ifdef SHADER_DEBUG
        vertex_data = std::vector<vertex_t>(data.begin(), data.end());
#endif

        glBindBuffer(GL_ARRAY_BUFFER, vbo_id);
        glBufferSubData(
          GL_ARRAY_BUFFER, 0, sizeof(vertex_t) * data.size(), data.data());
    }

    void draw(GLenum mode, GLuint num_elements) const {
        glBindVertexArray(vao_id);
        if (ebo_id == static_cast<GLuint>(-1)) {
            SDL_assert(num_indices_ == static_cast<GLuint>(-1));
            SDL_assert(num_elements <= num_vertices_);
            glDrawArrays(mode, 0, num_elements);
        } else {
            SDL_assert(num_elements <= num_indices_);
            glDrawElements(mode, num_elements, GL_UNSIGNED_INT, 0);
        }
    }

    void draw(GLenum mode) const {
        glBindVertexArray(vao_id);
        if (ebo_id == static_cast<GLuint>(-1)) {
            SDL_assert(num_indices_ == static_cast<GLuint>(-1));
            glDrawArrays(mode, 0, num_vertices_);
        } else {
            glDrawElements(mode, num_indices_, GL_UNSIGNED_INT, 0);
        }
    }

    // void draw(GLenum mode, const GLsizei num_indices,
    //           const uint indices[num_indices]) {
    //     glBindVertexArray(vao_id);
    //     glDrawElements(mode, num_indices, GL_UNSIGNED_INT,
    //                    (const void*)indices);
    // }
};
