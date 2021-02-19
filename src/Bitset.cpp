#pragma once
#include "Bitset.h"
#include "Util.h"
#include <sdl/SDL_assert.h>

void Bitset::init(size_t size_) {
    SDL_assert(mask == nullptr && size == 0);

    size = size_;

    size_t bit_array_length = size / BITS_PER_ELEM;
    if (size % BITS_PER_ELEM != 0)
        bit_array_length++;

#ifdef _DEBUG
    mask_array_size = bit_array_length;
#endif

    mask = new u8[bit_array_length];
    memset(mask, 0, sizeof(*mask) * bit_array_length);
}

void Bitset::destroy() {
    SDL_assert(mask && size != 0);

    size = 0;
    delete[] mask;
    mask = nullptr;
}

void Bitset::clear() {
    if (mask)
        memset(mask, 0, sizeof(*mask) * (size / BITS_PER_ELEM));
}

template <typename T> static constexpr size_t size_in_bits(T elem) {
    return sizeof(elem) * 8;
}

void Bitset::set(size_t n) {
    SDL_assert(n < size);

    size_t array_index = n / BITS_PER_ELEM;

#ifdef _DEBUG
    SDL_assert(array_index < mask_array_size);
#endif

    mask[array_index] |= BIT(n % BITS_PER_ELEM);
}

void Bitset::unset(size_t n) {
    SDL_assert(n < size);

    size_t array_index = n / BITS_PER_ELEM;

#ifdef _DEBUG
    SDL_assert(array_index < mask_array_size);
#endif

    mask[array_index] &= ~BIT(n % BITS_PER_ELEM);
}

bool Bitset::is_set(size_t n) const {
    SDL_assert(n < size);

    size_t array_index = n / BITS_PER_ELEM;

#ifdef _DEBUG
    SDL_assert(array_index < mask_array_size);
#endif

    return mask[array_index] & BIT(n % BITS_PER_ELEM);
}

size_t Bitset::find_first_set_bit(size_t start_index) const {
    SDL_assert(start_index < size);
    u8* runner = mask + start_index;

    for (size_t n_bit = start_index; n_bit < size; ++n_bit) {
        size_t n_bit_in_current_array_elem = n_bit % BITS_PER_ELEM;

        if (*runner & BIT(n_bit_in_current_array_elem))
            return n_bit;

        if (n_bit_in_current_array_elem == BITS_PER_ELEM - 1)
            runner++;
    }
    return static_cast<size_t>(-1);
}

size_t Bitset::find_first_unset_bit() const {
    // NOTE: There's probably some really advanced way to do this by ANDing mask
    // with a bit field consisting of only 1s, bot we won't do that here.

    u8* runner = mask;
    for (size_t n_bit = 0; n_bit < size; ++n_bit) {
        size_t n_bit_in_current_array_elem = n_bit % BITS_PER_ELEM;

        if (!(*runner & BIT(n_bit_in_current_array_elem)))
            return n_bit;

        if (n_bit_in_current_array_elem == BITS_PER_ELEM - 1)
            runner++;
    }
    SDL_TriggerBreakpoint();
    return static_cast<size_t>(-1);
}
