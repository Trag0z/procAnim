#pragma once
#include "pch.h"

constexpr float PI = 3.14159265358979323846f;

#define BIT(X) (1 << X)

inline void setNthBitTo(unsigned int& bitField, unsigned int n, int value) {
    bitField ^= (-value ^ bitField) & (1u << n);
}

inline float degToRad(float degrees) { return degrees * PI / 180.0f; }

inline float radToDeg(float radians) { return radians * 180.0f / PI; }

void printGlm(const glm::vec4& v);
void printGlm(const glm::mat4& m);

template <typename T> void printGlm(const char* title, const T& t) {
    printf("%s\n", title);
    printGlm(t);
}

template <typename T> T lerp(T a, T b, float t) { return a + (b - a) * t; }

inline float clamp(float val, float min, float max) {
    if (val < min) {
        return min;
    } else if (val > max) {
        return max;
    } else {
        return val;
    }
}

typedef const wchar_t* cwstrptr_t;
bool get_save_path(std::string& path, cwstrptr_t filter_name = nullptr,
                   cwstrptr_t filter_pattern = nullptr,
                   cwstrptr_t default_extension = nullptr);
bool get_load_path(std::string& path, cwstrptr_t filter_name = nullptr,
                   cwstrptr_t filter_pattern = nullptr);