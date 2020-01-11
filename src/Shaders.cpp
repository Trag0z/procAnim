#pragma once
#include "Shaders.h"
#include <fstream>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <sdl/SDL_assert.h>
#include <sstream>

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
	}
	else {
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
	}
	catch (std::exception e) {
		printf("ERROR: Failed to read shader file/n");
		SDL_assert_always(false);
	}

	const GLchar* vShaderSource = vShaderString.c_str();
	const GLchar* fShaderSource = fShaderString.c_str();

	GLuint vShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vShader, 1, &vShaderSource, NULL);
	glCompileShader(vShader);
	if (!checkCompileErrors(vShader, false))
		return -1;

	GLuint fShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fShader, 1, &fShaderSource, NULL);
	glCompileShader(fShader);
	if (!checkCompileErrors(fShader, false))
		return -1;

	GLuint id = glCreateProgram();

	glAttachShader(id, vShader);
	glAttachShader(id, fShader);

	glLinkProgram(id);
	if (!checkCompileErrors(id, true))
		return -1;

	glDeleteShader(vShader);
	glDeleteShader(fShader);

	return id;
}