#pragma once
#include "pch.h"
#include "Color.h"
#include "Texture.h"
#include "VertexArray.h"
#include "Types.h"

static bool check_compile_errors(GLuint object, bool program);

static GLuint load_and_compile_shader_from_file(const char* vert_shader_path,
                                                const char* frag_shader_path);

namespace ShaderDetail {
// This class should never be instantiated, only derived from.
class Shader {
  protected:
    GLuint id;
    GLuint camera_loc, model_loc;

    Shader() {}
    Shader(const char* vert_path, const char* frag_path);

  public:
    void use() const;
    void set_camera(const glm::mat3* mat) const;
    void set_model(const glm::mat3* mat) const;
};
} // namespace ShaderDetail

class DebugShader : public ShaderDetail::Shader {
    GLuint color_loc;

  public:
    DebugShader() {}
    DebugShader(const char* vert_path, const char* frag_path);

    void set_color(const Color* color) const;

    struct Vertex {
        glm::vec2 pos;
    };

    static VertexArray<Vertex> DEFAULT_VAO;
};

class TexturedShader : public ShaderDetail::Shader {
  public:
    TexturedShader() {}
    TexturedShader(const char* vert_path, const char* frag_path);

    void set_texture(const Texture& texture) const;

    struct Vertex {
        glm::vec2 pos;
        glm::vec2 uv_coord;
    };

    static VertexArray<Vertex> DEFAULT_VAO;
};

class RiggedShader : public ShaderDetail::Shader {
    GLuint bone_transforms_loc;

  public:
    static const size_t NUMBER_OF_BONES = 15;
    static const size_t MAX_BONES_PER_VERTEX = 2;

    RiggedShader() {}
    RiggedShader(const char* vert_path, const char* frag_path);

    void set_bone_transforms(const glm::mat3* transforms) const;

    struct Vertex {
        glm::vec2 pos;
        glm::vec2 uv_coord;
        uint bone_indices[MAX_BONES_PER_VERTEX];
        float bone_weights[MAX_BONES_PER_VERTEX];
    };
};

//                                                              //
//          Template specifications for VertexArray<>           //
//                                                              //

void VertexArray<RiggedShader::Vertex>::init(
    const GLuint* indices, GLuint num_indices,
    const RiggedShader::Vertex* vertices, GLuint num_vertices, GLenum usage) {
    num_indices_ = num_indices;
    num_vertices_ = num_vertices;
    usage_ = usage;

    glGenVertexArrays(1, &vao_id);
    glBindVertexArray(vao_id);

    // Create index buffer
    glGenBuffers(1, &ebo_id);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_id);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * num_indices_,
                 indices, usage);

    // Create vertex buffer
    glGenBuffers(1, &vbo_id);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_id);
    glBufferData(GL_ARRAY_BUFFER, sizeof(RiggedShader::Vertex) * num_vertices_,
                 vertices, usage);

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
    // bone_indices attribute
    glVertexAttribIPointer(
        2, 2, GL_UNSIGNED_INT, sizeof(RiggedShader::Vertex),
        reinterpret_cast<void*>(offsetof(RiggedShader::Vertex, bone_indices)));
    glEnableVertexAttribArray(2);
    // bone_weights attribute
    glVertexAttribPointer(
        3, 2, GL_FLOAT, GL_FALSE, sizeof(RiggedShader::Vertex),
        reinterpret_cast<void*>(offsetof(RiggedShader::Vertex, bone_weights)));
    glEnableVertexAttribArray(3);
}

void VertexArray<TexturedShader::Vertex>::init(
    const GLuint* indices, GLuint num_indices,
    const TexturedShader::Vertex* vertices, GLuint num_vertices, GLenum usage) {
    num_indices_ = num_indices;
    num_vertices_ = num_vertices;
    usage_ = usage;

    glGenVertexArrays(1, &vao_id);
    glBindVertexArray(vao_id);

    // Create index buffer
    glGenBuffers(1, &ebo_id);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_id);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * num_indices_,
                 indices, usage);

    // Create vertex buffer
    glGenBuffers(1, &vbo_id);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_id);
    glBufferData(GL_ARRAY_BUFFER,
                 sizeof(TexturedShader::Vertex) * num_vertices_, vertices,
                 usage);

    // position attribute
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE,
                          sizeof(TexturedShader::Vertex),
                          reinterpret_cast<void*>(0));
    glEnableVertexAttribArray(0);
    // uvCoord attribute
    glVertexAttribPointer(
        1, 2, GL_FLOAT, GL_FALSE, sizeof(TexturedShader::Vertex),
        reinterpret_cast<void*>(offsetof(TexturedShader::Vertex, uv_coord)));
    glEnableVertexAttribArray(1);
}

void VertexArray<DebugShader::Vertex>::init(const GLuint* indices,
                                            GLuint num_indices,
                                            const DebugShader::Vertex* vertices,
                                            GLuint num_vertices, GLenum usage) {
    num_indices_ = num_indices;
    num_vertices_ = num_vertices;
    usage_ = usage;

    glGenVertexArrays(1, &vao_id);
    glBindVertexArray(vao_id);

    // Create index buffer
    glGenBuffers(1, &ebo_id);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_id);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * num_indices, indices,
                 usage);

    // Create vertex buffer
    glGenBuffers(1, &vbo_id);

    glBindBuffer(GL_ARRAY_BUFFER, vbo_id);
    glBufferData(GL_ARRAY_BUFFER, sizeof(DebugShader::Vertex) * num_vertices,
                 vertices, usage);

    // position attribute
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(DebugShader::Vertex),
                          reinterpret_cast<void*>(0));
    glEnableVertexAttribArray(0);
}