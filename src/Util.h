#pragma once
#include "pch.h"

constexpr float PI = 3.14159265358979323846f;

#define BIT(X) (1 << X)

inline void setNthBitTo(unsigned int& bitField, unsigned int n, int value) {
    bitField ^= (-value ^ bitField) & (1u << n);
};

inline float degToRad(float degrees) { return degrees * PI / 180; }

void printGlm(const glm::vec4& v);
void printGlm(const glm::mat4& m);

template <typename T> void printGlm(const char* title, const T& t) {
    printf("%s\n", title);
    printGlm(t);
}
