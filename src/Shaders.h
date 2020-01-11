#pragma once
#include "pch.h"


static bool checkCompileErrors(GLuint object, bool program);

// Returns -1 on error
GLuint loadAndCompileShaderFromFile(const char* vShaderPath,
	const char* fShaderPath);