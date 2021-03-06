#pragma once
#include <string>

constexpr float PI = 3.14159265358979323846f;

#define BIT(X) (1u << X)

inline void setNthBitTo(uint& bitField, uint n, int value) {
    bitField ^= (-value ^ bitField) & (1u << n);
}

template<typename T>
T lerp(T a, T b, float t) {
    return a + (b - a) * t;
}

float length_squared(glm::vec2 v);

typedef const wchar_t* cwstrptr_t;
bool get_save_path(std::string& path,
                   cwstrptr_t filter_name       = nullptr,
                   cwstrptr_t filter_pattern    = nullptr,
                   cwstrptr_t default_extension = nullptr);
bool get_load_path(std::string& path,
                   cwstrptr_t filter_name    = nullptr,
                   cwstrptr_t filter_pattern = nullptr);