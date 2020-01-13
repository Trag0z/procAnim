#pragma once
#include "pch.h"

#define BIT(X) (1 << X)

inline void setNthBitTo(unsigned int& bitField, unsigned int n, int value) {
    bitField ^= (-value ^ bitField) & (1u << n);
};

void printGlm(const glm::vec4& v);
void printGlm(const glm::mat4& m);

template <typename T> void printGlm(const char* title, const T& t) {
    printf("%s\n", title);
    printGlm(t);
}