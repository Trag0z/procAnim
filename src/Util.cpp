#pragma once
#include "pch.h"
#include "Util.h"

void printGlm(const glm::vec4& v) {
    printf("%.4f, %.4f, %.4f, %.4f\n", v[0], v[1], v[2], v[3]);
}

void printGlm(const glm::mat4& m) {
    printf("%.4f, %.4f, %.4f, %.4f\n%.4f, %.4f, %.4f, %.4f\n%.4f, "
           "%.4f, %.4f, %.4f\n%.4f, %.4f, %.4f, %.4f\n",
           m[0][0], m[1][0], m[2][0], m[3][0], m[0][1], m[1][1], m[2][1],
           m[3][1], m[0][2], m[1][2], m[2][2], m[3][2], m[0][3], m[1][3],
           m[2][3], m[3][3]);
}