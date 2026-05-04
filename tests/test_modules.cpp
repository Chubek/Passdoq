#include <catch2/catch_test_macros.hpp>
#include "../src/modules.hpp"
#include "../src/crypto.hpp"

using namespace passdoq::modules;

TEST_CASE("Module manager initialization", "[modules]") {
    REQUIRE(passdoq::crypto::init());
    
    ModuleManager manager;
    
    SECTION("List modules when empty") {
        auto modules = manager.list_modules();
        REQUIRE(modules.empty());
    }
    
    SECTION("Check non-existent module") {
        REQUIRE_FALSE(manager.is_loaded("nonexistent"));
    }
}

TEST_CASE("Module loading", "[modules]") {
    REQUIRE(passdoq::crypto::init());
    
    ModuleManager manager;
    
    SECTION("Load non-existent module fails") {
        REQUIRE_FALSE(manager.load_module("/nonexistent/module.so"));
    }
    
    SECTION("Unload non-existent module fails") {
        REQUIRE_FALSE(manager.unload_module("nonexistent"));
    }
}

TEST_CASE("Module context", "[modules]") {
    ModuleContext ctx;
    ctx.vault = nullptr;
    ctx.user_data = nullptr;
    
    REQUIRE(ctx.vault == nullptr);
    REQUIRE(ctx.user_data == nullptr);
}
