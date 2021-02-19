#pragma once
#include "Types.h"

class Bitset {
    size_t size = 0;
    u8* mask = nullptr;
#ifdef _DEBUG
    size_t mask_array_size = 0;
#endif

    const size_t BITS_PER_ELEM = sizeof(*mask) * 8;

  public:
    void init(size_t size_);
    void destroy();
    void clear();

    void set(size_t n);
    void unset(size_t n);

    bool is_set(size_t n) const;

    size_t find_first_set_bit(size_t start_index = 0) const;
    size_t find_first_unset_bit() const;
};