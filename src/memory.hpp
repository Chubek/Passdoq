#ifndef PASSDOQ_MEMORY_HPP
#define PASSDOQ_MEMORY_HPP

#include <cstdint>
#include <memory>
#include <vector>
#include <chrono>

namespace passdoq {
namespace memory {

// Secure allocator for sensitive data
template<typename T>
class SecureAllocator {
public:
    using value_type = T;
    using pointer = T*;
    using const_pointer = const T*;
    using size_type = std::size_t;
    
    SecureAllocator() noexcept = default;
    
    template<typename U>
    SecureAllocator(const SecureAllocator<U>&) noexcept {}
    
    pointer allocate(size_type n);
    void deallocate(pointer p, size_type n) noexcept;
    
    template<typename U>
    struct rebind {
        using other = SecureAllocator<U>;
    };
};

template<typename T, typename U>
bool operator==(const SecureAllocator<T>&, const SecureAllocator<U>&) { return true; }

template<typename T, typename U>
bool operator!=(const SecureAllocator<T>&, const SecureAllocator<U>&) { return false; }

// Secure vector type
template<typename T>
using SecureVector = std::vector<T, SecureAllocator<T>>;

// Secure buffer that auto-zeros on destruction
class SecureBuffer {
public:
    SecureBuffer(size_t size);
    ~SecureBuffer();
    
    // No copy
    SecureBuffer(const SecureBuffer&) = delete;
    SecureBuffer& operator=(const SecureBuffer&) = delete;
    
    // Move allowed
    SecureBuffer(SecureBuffer&& other) noexcept;
    SecureBuffer& operator=(SecureBuffer&& other) noexcept;
    
    uint8_t* data() { return buffer_.data(); }
    const uint8_t* data() const { return buffer_.data(); }
    size_t size() const { return buffer_.size(); }
    
    void zero();
    
private:
    std::vector<uint8_t> buffer_;
};

// Time-limited secure storage
class TimeLimitedBuffer {
public:
    TimeLimitedBuffer(size_t size, std::chrono::seconds timeout);
    ~TimeLimitedBuffer();
    
    uint8_t* data();
    const uint8_t* data() const;
    size_t size() const;
    
    bool is_expired() const;
    void refresh_timeout();
    void lock();
    
private:
    SecureBuffer buffer_;
    std::chrono::steady_clock::time_point expiry_;
    std::chrono::seconds timeout_;
    bool locked_;
};

// Initialize secure memory subsystem
bool init();

// Lock memory page (prevent swapping)
bool lock_memory(void* addr, size_t size);

// Unlock memory page
bool unlock_memory(void* addr, size_t size);

} // namespace memory
} // namespace passdoq

#endif // PASSDOQ_MEMORY_HPP
