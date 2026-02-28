#pragma once
#include <cstddef>
#include <new>
#include <utility>

class Arena {
public:
    explicit Arena(size_t size);
    ~Arena();

    void* allocate(size_t size, size_t alignment = alignof(std::max_align_t));
    void reset();

    template<typename T, typename... Args>
    T* create(Args&&... args);

private:
    size_t buffer_size_;
    size_t current_offset_;
    void* buffer_;
};