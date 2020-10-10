#pragma once
#include "pch.h"
#include "Shaders.h"

static bool checkCompileErrors(GLuint object, bool program) {
    GLint success;
    GLchar infoLog[1024];

    if (program) {
        glGetProgramiv(object, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(object, 1024, NULL, infoLog);
            printf("ERROR: Program link-time error:\n%s", infoLog);
            return false;
        }
    } else {
        glGetShaderiv(object, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(object, 1024, NULL, infoLog);
            printf("ERROR: Shader compile-time error:\n%s", infoLog);
            return false;
        }
    }
    return true;
}

// Returns -1 on error
GLuint loadAndCompileShaderFromFile(const char* vShaderPath,
                                    const char* fShaderPath) {
    std::string vShaderString;
    std::string fShaderString;
    try {
        std::ifstream vShaderFile(vShaderPath);
        std::ifstream fShaderFile(fShaderPath);

        std::stringstream vShaderStream;
        std::stringstream fShaderStream;

        vShaderStream << vShaderFile.rdbuf();
        fShaderStream << fShaderFile.rdbuf();

        vShaderFile.close();
        fShaderFile.close();

        vShaderString = vShaderStream.str();
        fShaderString = fShaderStream.str();
    } catch (std::exception e) {
        printf("ERROR: Failed to read shader file/n");
        SDL_assert_always(false);
    }

    const GLchar* vShaderSource = vShaderString.c_str();
    const GLchar* fShaderSource = fShaderString.c_str();

    GLuint vShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vShader, 1, &vShaderSource, NULL);
    glCompileShader(vShader);
    if (!checkCompileErrors(vShader, false))
        SDL_assert_always(false);

    GLuint fShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fShader, 1, &fShaderSource, NULL);
    glCompileShader(fShader);
    if (!checkCompileErrors(fShader, false))
        SDL_assert_always(false);

    GLuint id = glCreateProgram();

    glAttachShader(id, vShader);
    glAttachShader(id, fShader);

    glLinkProgram(id);
    if (!checkCompileErrors(id, true))
        SDL_assert_always(false);

    glDeleteShader(vShader);
    glDeleteShader(fShader);

    return id;
}

RiggedShader::RiggedShader(const char* vert_path, const char* frag_path)
    : Shader(vert_path, frag_path) {
    bone_transforms_loc = glGetUniformLocation(id, "bone_transforms[0]");
}

void RiggedShader::set_bone_transforms(const glm::mat3* transforms) const {
    use();
    glUniformMatrix3fv(bone_transforms_loc, NUMBER_OF_BONES, 0,
                       (const GLfloat*)transforms);
}

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

VertexArray<DebugShader::Vertex> DebugShader::DEFAULT_VAO;

DebugShader::DebugShader(const char* vert_path, const char* frag_path)
    : Shader(vert_path, frag_path) {
    color_loc = glGetUniformLocation(id, "color");

    GLuint indices[6] = {0, 1, 2, 2, 3, 0};

    Vertex vertices[4];
    vertices[0] = {{-1.0f, 1.0f}};
    vertices[1] = {{-1.0f, -1.0f}};
    vertices[2] = {{1.0f, -1.0f}};
    vertices[3] = {{1.0f, 1.0f}};

    DEFAULT_VAO.init(indices, 6, vertices, 4, GL_STATIC_DRAW);
}

void DebugShader::set_color(const Color* color) const {
    use();
    glUniform4fv(color_loc, 1, (const GLfloat*)color);
}