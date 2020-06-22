#pragma once
#include "pch.h"
#include "Colors.h"

static bool checkCompileErrors(GLuint object, bool program);

GLuint loadAndCompileShaderFromFile(const char* vShaderPath,
                                    const char* fShaderPath);

namespace {
class Shader {
  protected:
    GLuint id;
    GLuint projection_loc, model_loc;

    Shader() {}
    Shader(const char* vert_path, const char* frag_path) {
        id = loadAndCompileShaderFromFile(vert_path, frag_path);

        projection_loc = glGetUniformLocation(id, "projection");
        model_loc = glGetUniformLocation(id, "model");
    }

  public:
    // @CLEANUP: use() is called in every single uniform setter every time
    inline void use() const noexcept { glUseProgram(id); }

    inline void set_projection(const glm::mat4* mat) const noexcept {
        use();
        glUniformMatrix4fv(projection_loc, 1, 0, (const GLfloat*)mat);
    }

    inline void set_model(const glm::mat4* mat) const noexcept {
        use();
        glUniformMatrix4fv(model_loc, 1, 0, (const GLfloat*)mat);
    }
};
} // namespace

class RiggedShader : public Shader {
  public:
    RiggedShader() {}
    RiggedShader(const char* vert_path, const char* frag_path)
        : Shader(vert_path, frag_path) {}

    struct Vertex {
        glm::vec4 pos;
        glm::vec2 uv_coord;
    };
};

class DebugShader : public Shader {
    GLuint color_loc;

  public:
    DebugShader() {}
    DebugShader(const char* vert_path, const char* frag_path)
        : Shader(vert_path, frag_path) {
        color_loc = glGetUniformLocation(id, "color");
    }

    inline void set_color(const glm::vec4* color) const noexcept {
        use();
        glUniform4fv(color_loc, 1, (const GLfloat*)color);
    }

    struct Vertex {
        glm::vec4 pos;
    };
};
