#include "memory.hpp"
#include "crypto.hpp"
#include <sodium.h>
#include <sys/mman.h>
#include <stdexcept>
#include <cstring>

namespace passdoq {
namespace memory {

// SecureAllocator implementation
template<typename T>
typename SecureAllocator<T>::pointer SecureAllocator<T>::allocate(size_type n) {
    if (n == 0) return nullptr;
    
    size_t bytes = n * sizeof(T);
    void* ptr = sodium_malloc(bytes);
    
    if (!ptr) {
        throw std::bad_alloc();
    }
    
    // Lock memory to prevent swapping
    sodium_mlock(ptr, bytes);
    
    return static_cast<pointer>(ptr);
}

template<typename T>
void SecureAllocator<T>::deallocate(pointer p, size_type n) noexcept {
    if (p) {
        size_t bytes = n * sizeof(T);
        sodium_munlock(p, bytes);
        sodium_free(p);
    }
}

// Explicit instantiations
template class SecureAllocator<uint8_t>;
template class SecureAllocator<char>;

// SecureBuffer implementation
SecureBuffer::SecureBuffer(size_t size) : buffer_(size) {
    if (size > 0) {
        mlock(buffer_.data(), size);
    }
}

SecureBuffer::~SecureBuffer() {
    zero();
    if (!buffer_.empty()) {
        munlock(buffer_.data(), buffer_.size());
    }
}

SecureBuffer::SecureBuffer(SecureBuffer&& other) noexcept 
    : buffer_(std::move(other.buffer_)) {
}

SecureBuffer& SecureBuffer::operator=(SecureBuffer&& other) noexcept {
    if (this != &other) {
        zero();
        if (!buffer_.empty()) {
            munlock(buffer_.data(), buffer_.size());
        }
        buffer_ = std::move(other.buffer_);
    }
    return *this;
}

void SecureBuffer::zero() {
    if (!buffer_.empty()) {
        crypto::secure_zero(buffer_.data(), buffer_.size());
    }
}

// TimeLimitedBuffer implementation
TimeLimitedBuffer::TimeLimitedBuffer(size_t size, std::chrono::seconds timeout)
    : buffer_(size)
    , expiry_(std::chrono::steady_clock::now() + timeout)
    , timeout_(timeout)
    , locked_(false) {
}

TimeLimitedBuffer::~TimeLimitedBuffer() {
    lock();
}

uint8_t* TimeLimitedBuffer::data() {
    if (locked_ || is_expired()) {
        throw std::runtime_error("Buffer is locked or expired");
    }
    return buffer_.data();
}

const uint8_t* TimeLimitedBuffer::data() const {
    if (locked_ || is_expired()) {
        throw std::runtime_error("Buffer is locked or expired");
    }
    return buffer_.data();
}

size_t TimeLimitedBuffer::size() const {
    return buffer_.size();
}

bool TimeLimitedBuffer::is_expired() const {
    return std::chrono::steady_clock::now() >= expiry_;
}

void TimeLimitedBuffer::refresh_timeout() {
    expiry_ = std::chrono::steady_clock::now() + timeout_;
}

void TimeLimitedBuffer::lock() {
    buffer_.zero();
    locked_ = true;
}

// Module initialization
bool init() {
    // Sodium init handles secure memory setup
    return true;
}

bool lock_memory(void* addr, size_t size) {
    return mlock(addr, size) == 0;
}

bool unlock_memory(void* addr, size_t size) {
    return munlock(addr, size) == 0;
}

} // namespace memory
} // namespace passdoq
