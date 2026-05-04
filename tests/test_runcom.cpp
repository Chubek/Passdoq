#include <catch2/catch_test_macros.hpp>
#include "../src/runcom.hpp"
#include <fstream>
#include <filesystem>

using namespace passdoq::runcom;

TEST_CASE("Config value types", "[runcom]") {
    SECTION("String value") {
        auto val = Value::from_string("test");
        REQUIRE(val.type == ValueType::STRING);
        REQUIRE(val.string_value == "test");
    }
    
    SECTION("Integer value") {
        auto val = Value::from_int(42);
        REQUIRE(val.type == ValueType::INTEGER);
        REQUIRE(val.int_value == 42);
    }
    
    SECTION("Boolean value") {
        auto val = Value::from_bool(true);
        REQUIRE(val.type == ValueType::BOOLEAN);
        REQUIRE(val.bool_value == true);
    }
    
    SECTION("List value") {
        std::vector<std::string> list = {"a", "b", "c"};
        auto val = Value::from_list(list);
        REQUIRE(val.type == ValueType::LIST);
        REQUIRE(val.list_value == list);
    }
}

TEST_CASE("Config basic operations", "[runcom]") {
    Config config;
    
    SECTION("Set and get string") {
        config.set_string("key", "value");
        REQUIRE(config.has("key"));
        REQUIRE(config.get_string("key") == "value");
    }
    
    SECTION("Set and get integer") {
        config.set_int("number", 123);
        REQUIRE(config.has("number"));
        REQUIRE(config.get_int("number") == 123);
    }
    
    SECTION("Set and get boolean") {
        config.set_bool("flag", true);
        REQUIRE(config.has("flag"));
        REQUIRE(config.get_bool("flag") == true);
    }
    
    SECTION("Set and get list") {
        std::vector<std::string> list = {"item1", "item2"};
        config.set_list("items", list);
        REQUIRE(config.has("items"));
        REQUIRE(config.get_list("items") == list);
    }
    
    SECTION("Default values") {
        REQUIRE(config.get_string("nonexistent", "default") == "default");
        REQUIRE(config.get_int("nonexistent", 99) == 99);
        REQUIRE(config.get_bool("nonexistent", true) == true);
    }
    
    SECTION("List all keys") {
        config.set_string("key1", "val1");
        config.set_int("key2", 42);
        
        auto keys = config.keys();
        REQUIRE(keys.size() == 2);
        REQUIRE(std::find(keys.begin(), keys.end(), "key1") != keys.end());
        REQUIRE(std::find(keys.begin(), keys.end(), "key2") != keys.end());
    }
}

TEST_CASE("Config file loading", "[runcom]") {
    const std::string test_file = "/tmp/test_passdoq.cnf";
    
    SECTION("Load simple config") {
        std::ofstream file(test_file);
        file << "vault_path = /home/user/.vault\n";
        file << "timeout = 300\n";
        file << "auto_lock = true\n";
        file.close();
        
        Config config;
        REQUIRE(config.load(test_file));
        
        REQUIRE(config.get_string("vault_path") == "/home/user/.vault");
        REQUIRE(config.get_int("timeout") == 300);
        REQUIRE(config.get_bool("auto_lock") == true);
    }
    
    SECTION("Load config with comments") {
        std::ofstream file(test_file);
        file << "# This is a comment\n";
        file << "key = value\n";
        file << "# Another comment\n";
        file.close();
        
        Config config;
        REQUIRE(config.load(test_file));
        REQUIRE(config.get_string("key") == "value");
    }
    
    SECTION("Load config with quoted strings") {
        std::ofstream file(test_file);
        file << "message = \"Hello World\"\n";
        file.close();
        
        Config config;
        REQUIRE(config.load(test_file));
        REQUIRE(config.get_string("message") == "Hello World");
    }
    
    std::filesystem::remove(test_file);
}

TEST_CASE("Config includes", "[runcom]") {
    const std::string main_file = "/tmp/test_main.cnf";
    const std::string include_file = "/tmp/test_include.cnf";
    
    // Create include file
    std::ofstream inc(include_file);
    inc << "included_key = included_value\n";
    inc.close();
    
    // Create main file with include
    std::ofstream main(main_file);
    main << "main_key = main_value\n";
    main << "#include \"" << include_file << "\"\n";
    main.close();
    
    Config config;
    REQUIRE(config.load(main_file));
    
    REQUIRE(config.get_string("main_key") == "main_value");
    REQUIRE(config.get_string("included_key") == "included_value");
    
    std::filesystem::remove(main_file);
    std::filesystem::remove(include_file);
}

TEST_CASE("Default config paths", "[runcom]") {
    auto paths = get_default_paths();
    
    REQUIRE(!paths.empty());
    
    // Should include ~/.passdoqrc
    bool has_home_rc = false;
    for (const auto& path : paths) {
        if (path.find(".passdoqrc") != std::string::npos) {
            has_home_rc = true;
            break;
        }
    }
    REQUIRE(has_home_rc);
}
