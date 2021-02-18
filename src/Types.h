#pragma once
#include <cstdint>
#include <gl/glew.h>
#include <glm/vec2.hpp>

// Some typedefs, makes using integer types of specific sizes more readable and
// easier to type. SDL specifically sometimes requires signed or unsigned 32 bit
// types.

typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef GLuint uint;
typedef glm::vec2 vec2;