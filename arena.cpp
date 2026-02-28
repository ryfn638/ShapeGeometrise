#include <cstddef>
#include <cstdint>
#include <new>
#include <arena.h>
using namespace std;

// Basics of memory arena,
// This is only used to store 

class Arena
{
public:
    explicit Arena(size_t size) : buffer_size_(size), current_offset_(0) {
        buffer_ = new std::byte[buffer_size_];
    }


    ~Arena() {
        delete[] buffer_;
    }


    Arena(const Arena&) = delete;
    Arena& operator=(const Arena&) = delete;

    // Allocate method: Returns a pointer to a block of memory
    void* allocate(size_t size, size_t alignment = alignof(std::max_align_t)) {
        // Calculate the aligned offset
        size_t aligned_offset = (current_offset_ + alignment - 1) & ~(alignment - 1);

        // Check if there is enough space left in the buffer
        if (aligned_offset + size > buffer_size_) {
            // In a real-world scenario, you might request a new, larger block
            // and chain it. For this simple example, we return nullptr.
            return nullptr; 
        }

        // Get a pointer to the newly allocated block
        void* ptr = buffer_ + aligned_offset;
        current_offset_ = aligned_offset + size;
        return ptr;
    }

    // Reset method: Resets the offset to zero, making the arena reusable
    void reset() {
        current_offset_ = 0;
    }

    // Template method to allocate and construct an object using placement new
    template<typename T, typename... Args>
    T* create(Args&&... args) {
        void* ptr = allocate(sizeof(T), alignof(T));
        if (ptr == nullptr) {
            return nullptr;
        }
        // Use placement new to construct the object in the allocated memory
        return new (ptr) T(std::forward<Args>(args)...);
    }

private:
    std::byte* buffer_;
    size_t buffer_size_;
    size_t current_offset_;
};
