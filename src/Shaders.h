#pragma once
#include "pch.h"
#include "Color.h"
#include "Texture.h"
#include "VertexArray.h"
#include "Types.h"

static bool checkCompileErrors(GLuint object, bool program);

GLuint loadAndCompileShaderFromFile(const char* vShaderPath,
                                    const char* fShaderPath);

namespace {
// The shader base class should never be accessed by anyone outside
// of this file. Only the shaders themselves inherit from it.
class Shader {
  protected:
    GLuint id;
    GLuint camera_loc, model_loc;

    Shader() {}
    Shader(const char* vert_path, const char* frag_path) {
        id = loadAndCompileShaderFromFile(vert_path, frag_path);

        camera_loc = glGetUniformLocation(id, "camera");
        model_loc = glGetUniformLocation(id, "model");
    }

  public:
    // @OPTIMIZATION: use() is called in every single uniform setter every time
    void use() const { glUseProgram(id); };

    void set_camera(const glm::mat3* mat) const {
        use();
        glUniformMatrix3fv(camera_loc, 1, 0, (const GLfloat*)mat);
    }

    inline void set_model(const glm::mat3* mat) const {
        use();
        glUniformMatrix3fv(model_loc, 1, 0, (const GLfloat*)mat);
    };
};
} // namespace

class RiggedShader : public Shader {
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

class TexturedShader : public Shader {
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

class DebugShader : public Shader {
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

void VertexArray<RiggedShader::Vertex>::init(
    const GLuint* indices, GLuint num_indices,
    const RiggedShader::Vertex* vertices, GLuint num_vertices, GLenum usage) {
    num_indices_ = num_indices;
    num_vertices_ = num_vertices;

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