#include <catch2/catch_test_macros.hpp>
#include "../src/delivery.hpp"
#include <filesystem>

using namespace passdoq::delivery;

TEST_CASE("Clipboard delivery", "[delivery]") {
    ClipboardDelivery delivery;
    
    std::vector<uint8_t> secret = {'t', 'e', 's', 't', '_', 's', 'e', 'c', 'r', 'e', 't'};
    
    REQUIRE(delivery.deliver(secret));
    REQUIRE(delivery.name() == "clipboard");
}

TEST_CASE("File delivery", "[delivery]") {
    const std::string test_file = "/tmp/test_delivery.txt";
    std::filesystem::remove(test_file);
    
    FileDelivery delivery(test_file);
    
    std::vector<uint8_t> secret = {'t', 'e', 's', 't'};
    
    REQUIRE(delivery.deliver(secret));
    REQUIRE(delivery.name() == "file");
    REQUIRE(std::filesystem::exists(test_file));
    
    std::filesystem::remove(test_file);
}

TEST_CASE("Stdout delivery", "[delivery]") {
    StdoutDelivery delivery;
    
    std::vector<uint8_t> secret = {'t', 'e', 's', 't'};
    
    REQUIRE(delivery.deliver(secret));
    REQUIRE(delivery.name() == "stdout");
}

TEST_CASE("Memory delivery", "[delivery]") {
    MemoryDelivery delivery;
    
    std::vector<uint8_t> secret = {'t', 'e', 's', 't', '_', 's', 'e', 'c', 'r', 'e', 't'};
    
    REQUIRE(delivery.deliver(secret));
    REQUIRE(delivery.name() == "memory");
    REQUIRE(delivery.size() == secret.size());
    
    // Verify data
    for (size_t i = 0; i < secret.size(); ++i) {
        REQUIRE(delivery.data()[i] == secret[i]);
    }
}

TEST_CASE("Delivery manager", "[delivery]") {
    DeliveryManager manager;
    
    manager.register_method(std::make_unique<StdoutDelivery>());
    manager.register_method(std::make_unique<ClipboardDelivery>());
    
    SECTION("List methods") {
        auto methods = manager.list_methods();
        REQUIRE(methods.size() == 2);
    }
    
    SECTION("Deliver via registered method") {
        std::vector<uint8_t> secret = {'t', 'e', 's', 't'};
        REQUIRE(manager.deliver("stdout", secret));
    }
    
    SECTION("Deliver via non-existent method") {
        std::vector<uint8_t> secret = {'t', 'e', 's', 't'};
        REQUIRE_FALSE(manager.deliver("nonexistent", secret));
    }
}
