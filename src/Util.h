#pragma once
#include "pch.h"

#define BIT(X) (1 << X)

inline void setNthBitTo(unsigned int& bitField, unsigned int n, int value) {
    bitField ^= (-value ^ bitField) & (1u << n);
};