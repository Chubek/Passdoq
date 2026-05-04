#include <catch2/catch_test_macros.hpp>
#include "../src/search.hpp"
#include "../src/indexing.hpp"
#include "../src/vault.hpp"
#include "../src/crypto.hpp"
#include <filesystem>

using namespace passdoq::search;
using namespace passdoq::indexing;
using namespace passdoq::vault;

TEST_CASE("Query builder", "[search]") {
    SECTION("Build simple term query") {
        QueryBuilder builder;
        builder.add_term("test");
        
        auto query = builder.build();
        REQUIRE(!query.empty());
    }
    
    SECTION("Build phrase query") {
        QueryBuilder builder;
        builder.add_phrase("test phrase");
        
        auto query = builder.build();
        REQUIRE(!query.empty());
    }
    
    SECTION("Build query with tag filter") {
        QueryBuilder builder;
        builder.add_term("test");
        builder.filter_tag("work");
        
        auto query = builder.build();
        REQUIRE(!query.empty());
    }
    
    SECTION("Build query with metadata filter") {
        QueryBuilder builder;
        builder.add_term("test");
        builder.filter_metadata("url", "example.com");
        
        auto query = builder.build();
        REQUIRE(!query.empty());
    }
}

TEST_CASE("Search engine operations", "[search]") {
    REQUIRE(passdoq::crypto::init());
    
    const std::string index_path = "/tmp/test_search_index.xapian";
    std::filesystem::remove_all(index_path);
    
    // Create and populate index
    {
        IndexManager index(index_path);
        REQUIRE(index.create());
        
        Entry entry1;
        entry1.id = "entry-1";
        entry1.name = "GitHub Account";
        entry1.username = "developer";
        entry1.tags = {"work", "dev"};
        
        Entry entry2;
        entry2.id = "entry-2";
        entry2.name = "Gmail Account";
        entry2.username = "user@gmail.com";
        entry2.tags = {"personal", "email"};
        
        Entry entry3;
        entry3.id = "entry-3";
        entry3.name = "AWS Console";
        entry3.username = "admin";
        entry3.tags = {"work", "cloud"};
        
        REQUIRE(index.index_entry(entry1));
        REQUIRE(index.index_entry(entry2));
        REQUIRE(index.index_entry(entry3));
    }
    
    SearchEngine search(index_path);
    REQUIRE(search.open());
    
    SECTION("Search by term") {
        auto results = search.search("GitHub");
        REQUIRE(results.size() >= 1);
        REQUIRE(results[0].entry_id == "entry-1");
    }
    
    SECTION("Search by tag") {
        auto results = search.search_by_tag("work");
        REQUIRE(results.size() == 2);
    }
    
    SECTION("Search with no results") {
        auto results = search.search("nonexistent");
        REQUIRE(results.empty());
    }
    
    SECTION("Search with max results limit") {
        auto results = search.search("Account", 1);
        REQUIRE(results.size() == 1);
    }
    
    std::filesystem::remove_all(index_path);
}

TEST_CASE("Search suggestions", "[search]") {
    REQUIRE(passdoq::crypto::init());
    
    const std::string index_path = "/tmp/test_search_suggest.xapian";
    std::filesystem::remove_all(index_path);
    
    // Create and populate index
    {
        IndexManager index(index_path);
        REQUIRE(index.create());
        
        Entry entry1;
        entry1.id = "entry-1";
        entry1.name = "GitHub";
        entry1.username = "user";
        
        Entry entry2;
        entry2.id = "entry-2";
        entry2.name = "GitLab";
        entry2.username = "user";
        
        REQUIRE(index.index_entry(entry1));
        REQUIRE(index.index_entry(entry2));
    }
    
    SearchEngine search(index_path);
    REQUIRE(search.open());
    
    auto suggestions = search.suggest("git", 5);
    REQUIRE(!suggestions.empty());
    
    std::filesystem::remove_all(index_path);
}

TEST_CASE("Search result relevance", "[search]") {
    REQUIRE(passdoq::crypto::init());
    
    const std::string index_path = "/tmp/test_search_relevance.xapian";
    std::filesystem::remove_all(index_path);
    
    // Create and populate index
    {
        IndexManager index(index_path);
        REQUIRE(index.create());
        
        Entry entry1;
        entry1.id = "entry-1";
        entry1.name = "Important Document";
        entry1.username = "user";
        
        Entry entry2;
        entry2.id = "entry-2";
        entry2.name = "Document";
        entry2.username = "user";
        
        REQUIRE(index.index_entry(entry1));
        REQUIRE(index.index_entry(entry2));
    }
    
    SearchEngine search(index_path);
    REQUIRE(search.open());
    
    auto results = search.search("Important Document");
    REQUIRE(!results.empty());
    
    // First result should have higher relevance
    if (results.size() > 1) {
        REQUIRE(results[0].relevance >= results[1].relevance);
    }
    
    std::filesystem::remove_all(index_path);
}
