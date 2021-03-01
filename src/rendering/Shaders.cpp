#pragma once
#include <fstream>
#include <sstream>
#include "Shaders.h"
#include "../Util.h"

static bool check_compile_errors(GLuint object, bool program) {
    GLint success;
    GLchar info_log[1024];

    if (program) {
        glGetProgramiv(object, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(object, 1024, NULL, info_log);
            printf("ERROR: Program link-time error:\n%s", info_log);
            return false;
        }
    } else {
        glGetShaderiv(object, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(object, 1024, NULL, info_log);
            printf("ERROR: Shader compile-time error:\n%s", info_log);
            return false;
        }
    }
    return true;
}

// Returns -1 on error
static GLuint load_and_compile_shader_from_file(const char* vert_shader_path,
                                                const char* frag_shader_path) {
    std::string vert_shader_string;
    std::string frag_shader_string;
    try {
        std::ifstream vert_shader_file(vert_shader_path);
        std::ifstream frag_shader_file(frag_shader_path);

        std::stringstream vert_shader_stream;
        std::stringstream frag_shader_stream;

        vert_shader_stream << vert_shader_file.rdbuf();
        frag_shader_stream << frag_shader_file.rdbuf();

        vert_shader_file.close();
        frag_shader_file.close();

        vert_shader_string = vert_shader_stream.str();
        frag_shader_string = frag_shader_stream.str();
    } catch (std::exception e) {
        printf("ERROR: Failed to read shader file/n");
        SDL_TriggerBreakpoint();
    }

    const GLchar* vert_shader_source = vert_shader_string.c_str();
    const GLchar* frag_shader_source = frag_shader_string.c_str();

    GLuint vert_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vert_shader, 1, &vert_shader_source, NULL);
    glCompileShader(vert_shader);
    SDL_assert_always(check_compile_errors(vert_shader, false));

    GLuint frag_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(frag_shader, 1, &frag_shader_source, NULL);
    glCompileShader(frag_shader);
    SDL_assert_always(check_compile_errors(frag_shader, false));

    GLuint id = glCreateProgram();

    glAttachShader(id, vert_shader);
    glAttachShader(id, frag_shader);

    glLinkProgram(id);
    SDL_assert_always(check_compile_errors(id, true));

    glDeleteShader(vert_shader);
    glDeleteShader(frag_shader);

    return id;
}

namespace ShaderDetail {
Shader::Shader(const char* vert_path, const char* frag_path) {
    id = load_and_compile_shader_from_file(vert_path, frag_path);

    camera_loc = glGetUniformLocation(id, "camera");
    model_loc = glGetUniformLocation(id, "model");
}

void Shader::use() const { glUseProgram(id); };

void Shader::set_camera(const glm::mat3* mat) const {
    use();
    glUniformMatrix3fv(camera_loc, 1, 0, (const GLfloat*)mat);
}

void Shader::set_model(const glm::mat3* mat) const {
    use();
    glUniformMatrix3fv(model_loc, 1, 0, (const GLfloat*)mat);
};
} // namespace ShaderDetail

//                  DebugShader                     //
VertexArray<DebugShader::Vertex> DebugShader::SQUARE_VAO;
VertexArray<DebugShader::Vertex> DebugShader::CIRCLE_VAO;

DebugShader::DebugShader(const char* vert_path, const char* frag_path)
    : Shader(vert_path, frag_path) {
    color_loc = glGetUniformLocation(id, "color");

    GLuint indices[6] = {0, 1, 2, 2, 3, 0};

    Vertex vertices[4];
    vertices[0] = glm::vec2(-1.0f, 1.0f);
    vertices[1] = glm::vec2(-1.0f, -1.0f);
    vertices[2] = glm::vec2(1.0f, -1.0f);
    vertices[3] = glm::vec2(1.0f, 1.0f);

    SQUARE_VAO.init(indices, 6, vertices, 4, GL_STATIC_DRAW);

    // Init render data for rendering circle
    constexpr size_t CIRCLE_SEGMENTS = 30;

    GLuint circle_indices[CIRCLE_SEGMENTS];
    DebugShader::Vertex circle_vertices[CIRCLE_SEGMENTS];

    for (size_t i = 0; i < CIRCLE_SEGMENTS; ++i) {
        circle_indices[i] = static_cast<GLuint>(i);

        float theta = static_cast<float>(i) /
                      static_cast<float>(CIRCLE_SEGMENTS) * 2.0f * PI;

        circle_vertices[i] = glm::vec2(cosf(theta), sinf(theta));
    }

    CIRCLE_VAO.init(circle_indices, CIRCLE_SEGMENTS, circle_vertices,
                    CIRCLE_SEGMENTS, GL_STATIC_DRAW);
}

void DebugShader::set_color(const Color& color) const {
    use();
    glUniform4fv(color_loc, 1, (const GLfloat*)&color);
}

//                  TexturedShader                  //
VertexArray<TexturedShader::Vertex> TexturedShader::DEFAULT_VAO;

TexturedShader::TexturedShader(const char* vert_path, const char* frag_path)
    : Shader(vert_path, frag_path) {
    GLuint indices[6] = {0, 1, 2, 2, 3, 0};

    Vertex vertices[4];
    vertices[0] = {{-1.0f, 1.0f}, {0.0f, 0.0f}};
    vertices[1] = {{-1.0f, -1.0f}, {0.0f, 1.0f}};
    vertices[2] = {{1.0f, -1.0f}, {1.0f, 1.0f}};
    vertices[3] = {{1.0f, 1.0f}, {1.0f, 0.0f}};

    DEFAULT_VAO.init(indices, 6, vertices, 4, GL_STATIC_DRAW);
}

void TexturedShader::set_texture(const Texture& texture) const {
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture.id);
}

//                  RiggedShader                    //
RiggedShader::RiggedShader(const char* vert_path, const char* frag_path)
    : Shader(vert_path, frag_path) {
    bone_transforms_loc = glGetUniformLocation(id, "bone_transforms[0]");
}

void RiggedShader::set_texture(const Texture& texture) const {
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture.id);
}

void RiggedShader::set_bone_transforms(const glm::mat3* transforms) const {
    use();
    glUniformMatrix3fv(bone_transforms_loc, NUMBER_OF_BONES, 0,
                       (const GLfloat*)transforms);
}

//                  RiggedDebugShader               //
RiggedDebugShader::RiggedDebugShader(const char* vert_path,
                                     const char* frag_path)
    : Shader(vert_path, frag_path) {
    bone_transforms_loc = glGetUniformLocation(id, "bone_transforms[0]");
    color_loc = glGetUniformLocation(id, "color");
}

void RiggedDebugShader::set_color(const Color& color) const {
    use();
    glUniform4fv(color_loc, 1, (const GLfloat*)&color);
}

void RiggedDebugShader::set_bone_transforms(const glm::mat3* transforms) const {
    use();
    glUniformMatrix3fv(bone_transforms_loc, NUMBER_OF_BONES, 0,
                       (const GLfloat*)transforms);
}

//                  BoneShader                      //
BoneShader::BoneShader(const char* vert_path, const char* frag_path)
    : Shader(vert_path, frag_path) {
    color_loc = glGetUniformLocation(id, "color");
    bone_transforms_loc = glGetUniformLocation(id, "bone_transforms[0]");
}

void BoneShader::set_color(const Color& color) const {
    use();
    glUniform4fv(color_loc, 1, (const GLfloat*)&color);
}

void BoneShader::set_bone_transforms(const glm::mat3* transforms) const {
    use();
    glUniformMatrix3fv(bone_transforms_loc, NUMBER_OF_BONES, 0,
                       (const GLfloat*)transforms);
}

//                  TrailShader                     //
TrailShader::TrailShader(const char* vert_path, const char* frag_path)
    : Shader(vert_path, frag_path) {
    first_color_loc = glGetUniformLocation(id, "old_color");
    last_color_loc = glGetUniformLocation(id, "recent_color");
}

void TrailShader::set_colors(const Color& old_color,
                             const Color& recent_color) {
    use();
    glUniform4fv(first_color_loc, 1, (const GLfloat*)&old_color);
    glUniform4fv(last_color_loc, 1, (const GLfloat*)&recent_color);
}
