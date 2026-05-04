#include <catch2/catch_test_macros.hpp>
#include "../src/indexing.hpp"
#include "../src/crypto.hpp"
#include <filesystem>

using namespace passdoq::indexing;
using namespace passdoq::vault;

TEST_CASE("Index creation and opening", "[indexing]") {
    REQUIRE(passdoq::crypto::init());
    
    const std::string index_path = "/tmp/test_index.xapian";
    
    // Clean up
    std::filesystem::remove_all(index_path);
    
    SECTION("Create new index") {
        IndexManager index(index_path);
        REQUIRE(index.create());
        REQUIRE(index.exists());
    }
    
    SECTION("Open existing index") {
        {
            IndexManager index(index_path);
            REQUIRE(index.create());
        }
        
        IndexManager index(index_path);
        REQUIRE(index.open());
    }
    
    std::filesystem::remove_all(index_path);
}

TEST_CASE("Index entry operations", "[indexing]") {
    REQUIRE(passdoq::crypto::init());
    
    const std::string index_path = "/tmp/test_index_ops.xapian";
    std::filesystem::remove_all(index_path);
    
    IndexManager index(index_path);
    REQUIRE(index.create());
    
    SECTION("Index single entry") {
        Entry entry;
        entry.id = "test-id-1";
        entry.name = "Test Entry";
        entry.username = "testuser";
        entry.tags = {"work", "important"};
        entry.metadata["url"] = "https://example.com";
        
        REQUIRE(index.index_entry(entry));
    }
    
    SECTION("Update entry") {
        Entry entry;
        entry.id = "test-id-2";
        entry.name = "Original Name";
        entry.username = "user1";
        
        REQUIRE(index.index_entry(entry));
        
        entry.name = "Updated Name";
        REQUIRE(index.update_entry(entry));
    }
    
    SECTION("Remove entry") {
        Entry entry;
        entry.id = "test-id-3";
        entry.name = "To Remove";
        entry.username = "user";
        
        REQUIRE(index.index_entry(entry));
        REQUIRE(index.remove_entry(entry.id));
    }
    
    std::filesystem::remove_all(index_path);
}

TEST_CASE("Index rebuild", "[indexing]") {
    REQUIRE(passdoq::crypto::init());
    
    const std::string index_path = "/tmp/test_index_rebuild.xapian";
    std::filesystem::remove_all(index_path);
    
    IndexManager index(index_path);
    REQUIRE(index.create());
    
    std::vector<Entry> entries;
    
    for (int i = 0; i < 5; ++i) {
        Entry entry;
        entry.id = "entry-" + std::to_string(i);
        entry.name = "Entry " + std::to_string(i);
        entry.username = "user" + std::to_string(i);
        entries.push_back(entry);
    }
    
    REQUIRE(index.rebuild_index(entries));
    
    std::filesystem::remove_all(index_path);
}

TEST_CASE("Index clear", "[indexing]") {
    REQUIRE(passdoq::crypto::init());
    
    const std::string index_path = "/tmp/test_index_clear.xapian";
    std::filesystem::remove_all(index_path);
    
    IndexManager index(index_path);
    REQUIRE(index.create());
    
    Entry entry;
    entry.id = "test-id";
    entry.name = "Test Entry";
    entry.username = "user";
    
    REQUIRE(index.index_entry(entry));
    REQUIRE(index.clear());
    REQUIRE(index.exists());
    
    std::filesystem::remove_all(index_path);
}
