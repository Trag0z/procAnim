#pragma once
#include "Bitset.h"
#include <iterator>
#include <cstddef>

namespace Impl {
template <typename T> struct MemArena_Iterator {
    using value_type = T;
    using difference_type = std::ptrdiff_t;
    using pointer = T*;
    using reference = T&;
    using iterator_category = std::forward_iterator_tag;

  private:
    pointer ptr;
    pointer const mem_block;
    const Bitset& used_slots;
    const size_t arena_size;

  public:
    MemArena_Iterator(pointer ptr_, pointer const mem_block_,
                      const Bitset& used_slots_, size_t arena_size_)
        : ptr(ptr_), mem_block(mem_block_), used_slots(used_slots_),
          arena_size(arena_size_) {}

    reference operator*() const { return *ptr; }
    pointer operator->() { return ptr; }

    MemArena_Iterator<T>& operator++() {
        size_t current_index = ptr - mem_block;
        size_t next_item_index = used_slots.find_first_set_bit(current_index);

        if (next_item_index == static_cast<size_t>(-1)) {
            ptr = &mem_block[arena_size];
        } else {
            ptr = &mem_block[next_item_index];
        }
        return *this;
    }
    MemArena_Iterator<T> operator++(int) {
        auto tmp = *this;
        ++(*this);
        return tmp;
    }

    friend bool operator==(const MemArena_Iterator<T>& a,
                           const MemArena_Iterator<T>& b) {
        return a.ptr == b.ptr;
    }
    friend bool operator!=(const MemArena_Iterator<T>& a,
                           const MemArena_Iterator<T>& b) {
        return a.ptr != b.ptr;
    }
};

template <typename T> struct MemArena_Const_Iterator {
    using value_type = T;
    using difference_type = std::ptrdiff_t;
    using pointer = const T*;
    using reference = const T&;
    using iterator_category = std::forward_iterator_tag;

  private:
    pointer ptr;
    value_type* mem_block;
    const Bitset& used_slots;

  public:
    MemArena_Const_Iterator(pointer ptr_, value_type* mem_block_,
                            Bitset& used_slots_)
        : ptr(ptr_), mem_block(mem_block_), used_slots(used_slots_) {}

    reference operator*() const { return *ptr; }
    pointer operator->() const { return ptr; }

    MemArena_Const_Iterator<T>& operator++() {
        size_t current_index = ptr - mem_block;
        size_t next_item_index = used_slots.find_first_set_bit(current_index);

        SDL_assert(next_item_index != -1);
        ptr = &mem_block[next_item_index];
        return *this;
    }
    MemArena_Const_Iterator<T> operator++(int) {
        auto tmp = *this;
        ++(*this);
        return tmp;
    }

    friend bool operator==(const MemArena_Const_Iterator<T>& a,
                           const MemArena_Const_Iterator<T>& b) {
        return a.ptr == b.ptr;
    }
    friend bool operator!=(const MemArena_Const_Iterator<T>& a,
                           const MemArena_Const_Iterator<T>& b) {
        return a.ptr != b.ptr;
    }
};
} // namespace Impl

// TODO: add pointer to last element so iterator++ can return early
template <typename value_t> class MemArena {
    value_t* mem_block = nullptr;
    size_t num_elems_ = 0;
    size_t size = 0;

    Bitset used_slots;

  public:
    using iterator = Impl::MemArena_Iterator<value_t>;
    using const_iterator = Impl::MemArena_Const_Iterator<value_t>;

    void init(size_t size_);
    void destroy();
    void clear();

    value_t* add(value_t& item);
    void remove(value_t* item);

    size_t num_elems() const noexcept { return num_elems_; }

    iterator begin_stuff() {
        return iterator(mem_block, mem_block, used_slots, size);
    }
    iterator end_stuff() {
        return iterator(mem_block + size, mem_block, used_slots, size);
    }
    const_iterator cbegin() const {
        auto ret = const_iterator(mem_block, mem_block, used_slots);
        return ret;
    }
    const_iterator cend() const {
        auto ret = const_iterator(&mem_block[size], mem_block, used_slots);
        return ret;
    }
};

template <typename value_t> auto begin(MemArena<value_t> arena) {
    return arena.begin_stuff();
}
template <typename value_t> auto end(MemArena<value_t> arena) {
    return arena.end_stuff();
}
template <typename value_t> auto cbegin(const MemArena<value_t> arena) {
    return arena.cbegin();
}
template <typename value_t> auto cend(const MemArena<value_t> arena) {
    return arena.cend();
}

template <typename value_t> void MemArena<value_t>::init(size_t size_) {
    SDL_assert(mem_block == nullptr && size == 0);

    mem_block = new value_t[size];
    size = size_;
    used_slots.init(size);
};

template <typename value_t> void MemArena<value_t>::destroy() {
    SDL_assert(mem_block && size != 0);

    delete[] mem_block;
    size = 0;
    used_slots.destroy();
};

template <typename value_t> void MemArena<value_t>::clear() {
    num_elems_ = 0;
    used_slots.clear();
};

template <typename value_t> value_t* MemArena<value_t>::add(value_t& item) {
    size_t free_slot = used_slots.find_first_unset_bit();
    SDL_assert(free_slot != -1);

    ++num_elems_;
    SDL_assert(num_elems_ < size);

    mem_block[free_slot] = item;
    used_slots.set(free_slot);
    return &mem_block[free_slot];
};

template <typename value_t> void MemArena<value_t>::remove(value_t* item) {
    SDL_assert(item >= mem_block && item < mem_block + size);

    size_t item_index = (item - mem_block);
    SDL_assert(used_slots.is_set(item_index));
    used_slots.unset(item_index);
    --num_elems_;

    // TODO: remove
    SDL_assert(&mem_block[item_index] == item);
};