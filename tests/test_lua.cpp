#include <catch2/catch_test_macros.hpp>
#include "../src/lua.hpp"
#include "../src/crypto.hpp"

using namespace passdoq::lua;

TEST_CASE("Lua engine initialization", "[lua]") {
    REQUIRE(passdoq::crypto::init());
    
    LuaEngine engine;
    REQUIRE(engine.init());
}

TEST_CASE("Lua script execution", "[lua]") {
    REQUIRE(passdoq::crypto::init());
    
    LuaEngine engine;
    REQUIRE(engine.init());
    
    SECTION("Execute simple script") {
        REQUIRE(engine.load_string("x = 42"));
    }
    
    SECTION("Execute function") {
        REQUIRE(engine.load_string("function test_func() return 123 end"));
        REQUIRE(engine.call_function("test_func"));
    }
    
    SECTION("Invalid script fails") {
        REQUIRE_FALSE(engine.load_string("invalid lua syntax @#$"));
    }
}

TEST_CASE("Lua addon system", "[lua]") {
    REQUIRE(passdoq::crypto::init());
    
    LuaEngine engine;
    REQUIRE(engine.init());
    
    SECTION("Initialize addon") {
        std::string script = R"(
            local addon = lpassdoq.init_addon({
                name = "TestAddon",
                author = "Test Author",
                version = "1.0.0"
            })
        )";
        
        REQUIRE(engine.load_string(script));
        
        auto addons = engine.list_addons();
        REQUIRE(addons.size() == 1);
        REQUIRE(addons[0].name == "TestAddon");
        REQUIRE(addons[0].author == "Test Author");
    }
}

TEST_CASE("Lua event system", "[lua]") {
    REQUIRE(passdoq::crypto::init());
    
    LuaEngine engine;
    REQUIRE(engine.init());
    
    SECTION("Register and trigger event") {
        std::string script = R"(
            local addon = lpassdoq.init_addon({name = "EventTest"})
            
            _test_triggered = false
            
            addon.on("test_event", function()
                _test_triggered = true
            end)
        )";
        
        REQUIRE(engine.load_string(script));
        engine.trigger_event("test_event");
    }
}

TEST_CASE("Lua crypto bindings", "[lua]") {
    REQUIRE(passdoq::crypto::init());
    
    LuaEngine engine;
    REQUIRE(engine.init());
    
    SECTION("Generate random bytes") {
        std::string script = R"(
            local bytes = lpassdoq.random_bytes(16)
            assert(#bytes == 16)
        )";
        
        REQUIRE(engine.load_string(script));
    }
}
