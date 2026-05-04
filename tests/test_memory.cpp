#include <catch2/catch_test_macros.hpp>
#include "../src/memory.hpp"
#include "../src/crypto.hpp"
#include <thread>
#include <chrono>

using namespace passdoq::memory;

TEST_CASE("Memory initialization", "[memory]") {
    REQUIRE(passdoq::crypto::init());
    REQUIRE(init());
}

TEST_CASE("SecureBuffer basic operations", "[memory]") {
    REQUIRE(passdoq::crypto::init());
    
    SECTION("Create and access buffer") {
        SecureBuffer buffer(64);
        
        REQUIRE(buffer.size() == 64);
        REQUIRE(buffer.data() != nullptr);
    }
    
    SECTION("Zero buffer") {
        SecureBuffer buffer(32);
        
        // Fill with data
        for (size_t i = 0; i < buffer.size(); ++i) {
            buffer.data()[i] = static_cast<uint8_t>(i);
        }
        
        // Zero it
        buffer.zero();
        
        // Verify all zeros
        for (size_t i = 0; i < buffer.size(); ++i) {
            REQUIRE(buffer.data()[i] == 0);
        }
    }
    
    SECTION("Move semantics") {
        SecureBuffer buffer1(32);
        buffer1.data()[0] = 42;
        
        SecureBuffer buffer2(std::move(buffer1));
        
        REQUIRE(buffer2.size() == 32);
        REQUIRE(buffer2.data()[0] == 42);
    }
}

TEST_CASE("TimeLimitedBuffer operations", "[memory]") {
    REQUIRE(passdoq::crypto::init());
    
    SECTION("Create and access within timeout") {
        TimeLimitedBuffer buffer(64, std::chrono::seconds(5));
        
        REQUIRE(buffer.size() == 64);
        REQUIRE(!buffer.is_expired());
        REQUIRE(buffer.data() != nullptr);
    }
    
    SECTION("Buffer expires after timeout") {
        TimeLimitedBuffer buffer(64, std::chrono::milliseconds(100));
        
        REQUIRE(!buffer.is_expired());
        
        std::this_thread::sleep_for(std::chrono::milliseconds(150));
        
        REQUIRE(buffer.is_expired());
        REQUIRE_THROWS(buffer.data());
    }
    
    SECTION("Refresh timeout") {
        TimeLimitedBuffer buffer(64, std::chrono::milliseconds(100));
        
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        buffer.refresh_timeout();
        
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        REQUIRE(!buffer.is_expired());
    }
    
    SECTION("Lock buffer") {
        TimeLimitedBuffer buffer(64, std::chrono::seconds(5));
        
        buffer.data()[0] = 42;
        buffer.lock();
        
        REQUIRE_THROWS(buffer.data());
    }
}

TEST_CASE("Memory locking", "[memory]") {
    std::vector<uint8_t> data(4096);
    
    SECTION("Lock and unlock memory") {
        bool locked = lock_memory(data.data(), data.size());
        REQUIRE(locked);
        
        bool unlocked = unlock_memory(data.data(), data.size());
        REQUIRE(unlocked);
    }
}
