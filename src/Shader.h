#pragma once
#include <fstream>
#include <gl/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <sdl/SDL_assert.h>
#include <sstream>

struct Shader {
    GLuint id;

    void use() const { glUseProgram(id); }

    void setFloat(const GLchar *name, GLfloat value, GLboolean useShader) {
        glUniform1f(glGetUniformLocation(id, name), value);
    }
    void setInteger(const GLchar *name, GLint value, GLboolean useShader) {
        glUniform1i(glGetUniformLocation(id, name), value);
    }
    void setVector2f(const GLchar *name, GLfloat x, GLfloat y,
                     GLboolean useShader) {
        glUniform2f(glGetUniformLocation(id, name), x, y);
    }
    void setVector2f(const GLchar *name, const glm::vec2 &value,
                     GLboolean useShader) {
        glUniform2f(glGetUniformLocation(id, name), value.x, value.y);
    }
    void setVector3f(const GLchar *name, GLfloat x, GLfloat y, GLfloat z,
                     GLboolean useShader) {
        glUniform3f(glGetUniformLocation(id, name), x, y, z);
    }
    void setVector3f(const GLchar *name, const glm::vec3 &value,
                     GLboolean useShader) {
        glUniform3f(glGetUniformLocation(id, name), value.x, value.y, value.z);
    }
    void setVector4f(const GLchar *name, GLfloat x, GLfloat y, GLfloat z,
                     GLfloat w, GLboolean useShader) {
        glUniform4f(glGetUniformLocation(id, name), x, y, z, w);
    }
    void setVector4f(const GLchar *name, const glm::vec4 &value,
                     GLboolean useShader) {
        glUniform4f(glGetUniformLocation(id, name), value.x, value.y, value.z,
                    value.w);
    }
    void setMatrix4(const GLchar *name, const glm::mat4 &matrix,
                    GLboolean useShader) {
        glUniformMatrix4fv(glGetUniformLocation(id, name), 1, GL_FALSE,
                           glm::value_ptr(matrix));
    }
};

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

inline bool loadAndCompileShaderFromFile(Shader &shader,
                                         const char *vShaderPath,
                                         const char *fShaderPath) {

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
        SDL_assert(false);
    }

    const GLchar *vShaderSource = vShaderString.c_str();
    const GLchar *fShaderSource = fShaderString.c_str();

    GLuint vShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vShader, 1, &vShaderSource, NULL);
    glCompileShader(vShader);
    if (!checkCompileErrors(vShader, false))
        return false;

    GLuint fShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fShader, 1, &fShaderSource, NULL);
    glCompileShader(fShader);
    if (!checkCompileErrors(fShader, false))
        return false;

    GLuint pid = glCreateProgram();
    glAttachShader(pid, vShader);
    glAttachShader(pid, fShader);

    glLinkProgram(pid);
    if (!checkCompileErrors(pid, true))
        return false;

    glDeleteShader(vShader);
    glDeleteShader(fShader);

    shader.id = pid;

    return true;
}