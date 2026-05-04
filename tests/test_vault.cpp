#include <catch2/catch_test_macros.hpp>
#include "../src/vault.hpp"
#include "../src/crypto.hpp"
#include <filesystem>

using namespace passdoq::vault;

TEST_CASE("Vault creation and opening", "[vault]") {
    REQUIRE(passdoq::crypto::init());
    
    const std::string test_path = "/tmp/test_vault.pdq";
    const std::string passphrase = "test_password_123";
    
    // Clean up any existing test vault
    std::filesystem::remove(test_path);
    
    SECTION("Create new vault") {
        Vault vault;
        
        REQUIRE(vault.state() == VaultState::UNINITIALIZED);
        REQUIRE(vault.create(test_path, passphrase));
        REQUIRE(vault.state() == VaultState::UNLOCKED);
        REQUIRE(vault.is_unlocked());
    }
    
    SECTION("Open existing vault") {
        // Create vault first
        {
            Vault vault;
            REQUIRE(vault.create(test_path, passphrase));
        }
        
        // Open it
        Vault vault;
        REQUIRE(vault.open(test_path));
        REQUIRE(vault.state() == VaultState::LOCKED);
    }
    
    SECTION("Unlock vault with correct passphrase") {
        // Create vault
        {
            Vault vault;
            REQUIRE(vault.create(test_path, passphrase));
        }
        
        // Open and unlock
        Vault vault;
        REQUIRE(vault.open(test_path));
        REQUIRE(vault.unlock(passphrase));
        REQUIRE(vault.is_unlocked());
    }
    
    SECTION("Unlock fails with wrong passphrase") {
        // Create vault
        {
            Vault vault;
            REQUIRE(vault.create(test_path, passphrase));
        }
        
        // Try to unlock with wrong passphrase
        Vault vault;
        REQUIRE(vault.open(test_path));
        REQUIRE_FALSE(vault.unlock("wrong_password"));
        REQUIRE(vault.state() == VaultState::LOCKED);
    }
    
    // Cleanup
    std::filesystem::remove(test_path);
}

TEST_CASE("Vault locking", "[vault]") {
    REQUIRE(passdoq::crypto::init());
    
    const std::string test_path = "/tmp/test_vault_lock.pdq";
    const std::string passphrase = "test_password_123";
    
    std::filesystem::remove(test_path);
    
    Vault vault;
    REQUIRE(vault.create(test_path, passphrase));
    REQUIRE(vault.is_unlocked());
    
    vault.lock();
    REQUIRE(vault.state() == VaultState::LOCKED);
    REQUIRE_FALSE(vault.is_unlocked());
    
    std::filesystem::remove(test_path);
}

TEST_CASE("Entry management", "[vault]") {
    REQUIRE(passdoq::crypto::init());
    
    const std::string test_path = "/tmp/test_vault_entries.pdq";
    const std::string passphrase = "test_password_123";
    
    std::filesystem::remove(test_path);
    
    Vault vault;
    REQUIRE(vault.create(test_path, passphrase));
    
    SECTION("Add entry") {
        Entry entry;
        entry.name = "Test Service";
        entry.username = "testuser";
        entry.secret = std::vector<uint8_t>{'p', 'a', 's', 's', 'w', 'o', 'r', 'd'};
        entry.tags = {"work", "important"};
        entry.metadata["url"] = "https://example.com";
        
        std::string id = vault.add_entry(entry);
        
        REQUIRE(!id.empty());
        
        auto retrieved = vault.get_entry(id);
        REQUIRE(retrieved.name == entry.name);
        REQUIRE(retrieved.username == entry.username);
        REQUIRE(retrieved.secret == entry.secret);
        REQUIRE(retrieved.tags == entry.tags);
    }
    
    SECTION("Update entry") {
        Entry entry;
        entry.name = "Original Name";
        entry.username = "user1";
        entry.secret = std::vector<uint8_t>{'p', 'a', 's', 's'};
        
        std::string id = vault.add_entry(entry);
        
        Entry updated;
        updated.name = "Updated Name";
        updated.username = "user2";
        updated.secret = std::vector<uint8_t>{'n', 'e', 'w', 'p', 'a', 's', 's'};
        
        REQUIRE(vault.update_entry(id, updated));
        
        auto retrieved = vault.get_entry(id);
        REQUIRE(retrieved.name == "Updated Name");
        REQUIRE(retrieved.username == "user2");
    }
    
    SECTION("Delete entry") {
        Entry entry;
        entry.name = "To Delete";
        entry.username = "user";
        entry.secret = std::vector<uint8_t>{'p', 'a', 's', 's'};
        
        std::string id = vault.add_entry(entry);
        REQUIRE(vault.delete_entry(id));
        REQUIRE_THROWS(vault.get_entry(id));
    }
    
    SECTION("List entries") {
        Entry entry1;
        entry1.name = "Service 1";
        entry1.username = "user1";
        entry1.secret = std::vector<uint8_t>{'p', 'a', 's', 's', '1'};
        
        Entry entry2;
        entry2.name = "Service 2";
        entry2.username = "user2";
        entry2.secret = std::vector<uint8_t>{'p', 'a', 's', 's', '2'};
        
        vault.add_entry(entry1);
        vault.add_entry(entry2);
        
        auto entries = vault.list_entries();
        REQUIRE(entries.size() == 2);
    }
    
    std::filesystem::remove(test_path);
}

TEST_CASE("Entry search", "[vault]") {
    REQUIRE(passdoq::crypto::init());
    
    const std::string test_path = "/tmp/test_vault_search.pdq";
    const std::string passphrase = "test_password_123";
    
    std::filesystem::remove(test_path);
    
    Vault vault;
    REQUIRE(vault.create(test_path, passphrase));
    
    // Add test entries
    Entry entry1;
    entry1.name = "GitHub Account";
    entry1.username = "developer";
    entry1.secret = std::vector<uint8_t>{'p', 'a', 's', 's', '1'};
    entry1.tags = {"work", "dev"};
    
    Entry entry2;
    entry2.name = "Gmail Account";
    entry2.username = "user@gmail.com";
    entry2.secret = std::vector<uint8_t>{'p', 'a', 's', 's', '2'};
    entry2.tags = {"personal", "email"};
    
    Entry entry3;
    entry3.name = "AWS Console";
    entry3.username = "admin";
    entry3.secret = std::vector<uint8_t>{'p', 'a', 's', 's', '3'};
    entry3.tags = {"work", "cloud"};
    
    vault.add_entry(entry1);
    vault.add_entry(entry2);
    vault.add_entry(entry3);
    
    SECTION("Search by name") {
        auto results = vault.search("GitHub");
        REQUIRE(results.size() == 1);
        REQUIRE(results[0].name == "GitHub Account");
    }
    
    SECTION("Search by username") {
        auto results = vault.search("gmail.com");
        REQUIRE(results.size() == 1);
        REQUIRE(results[0].username == "user@gmail.com");
    }
    
    SECTION("Search by tag") {
        auto results = vault.search_by_tag("work");
        REQUIRE(results.size() == 2);
    }
    
    std::filesystem::remove(test_path);
}

TEST_CASE("Vault persistence", "[vault]") {
    REQUIRE(passdoq::crypto::init());
    
    const std::string test_path = "/tmp/test_vault_persist.pdq";
    const std::string passphrase = "test_password_123";
    
    std::filesystem::remove(test_path);
    
    std::string entry_id;
    
    // Create vault and add entry
    {
        Vault vault;
        REQUIRE(vault.create(test_path, passphrase));
        
        Entry entry;
        entry.name = "Persistent Entry";
        entry.username = "testuser";
        entry.secret = std::vector<uint8_t>{'s', 'e', 'c', 'r', 'e', 't'};
        
        entry_id = vault.add_entry(entry);
        REQUIRE(vault.save());
    }
    
    // Open vault and verify entry persisted
    {
        Vault vault;
        REQUIRE(vault.open(test_path));
        REQUIRE(vault.unlock(passphrase));
        
        auto entry = vault.get_entry(entry_id);
        REQUIRE(entry.name == "Persistent Entry");
        REQUIRE(entry.username == "testuser");
        REQUIRE(entry.secret == std::vector<uint8_t>{'s', 'e', 'c', 'r', 'e', 't'});
    }
    
    std::filesystem::remove(test_path);
}

TEST_CASE("Key rotation", "[vault]") {
    REQUIRE(passdoq::crypto::init());
    
    const std::string test_path = "/tmp/test_vault_rekey.pdq";
    const std::string passphrase = "test_password_123";
    
    std::filesystem::remove(test_path);
    
    Vault vault;
    REQUIRE(vault.create(test_path, passphrase));
    
    // Add entry
    Entry entry;
    entry.name = "Test Entry";
    entry.username = "user";
    entry.secret = std::vector<uint8_t>{'p', 'a', 's', 's'};
    std::string id = vault.add_entry(entry);
    
    // Rotate data key
    auto new_key = passdoq::crypto::generate_data_key();
    REQUIRE(vault.rekey(new_key));
    
    // Verify entry still accessible
    auto retrieved = vault.get_entry(id);
    REQUIRE(retrieved.name == "Test Entry");
    
    std::filesystem::remove(test_path);
}
