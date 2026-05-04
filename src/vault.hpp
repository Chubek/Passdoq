#ifndef PASSDOQ_VAULT_HPP
#define PASSDOQ_VAULT_HPP

#include "crypto.hpp"
#include "memory.hpp"
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <chrono>

namespace passdoq {
namespace vault {

// Vault entry
struct Entry {
    std::string id;
    std::string name;
    std::string username;
    std::vector<uint8_t> secret;
    std::map<std::string, std::string> metadata;
    std::vector<std::string> tags;
    std::chrono::system_clock::time_point created;
    std::chrono::system_clock::time_point modified;
};

// Vault state
enum class VaultState {
    LOCKED,
    UNLOCKED,
    UNINITIALIZED
};

// Vault header (stored in plaintext)
struct VaultHeader {
    uint32_t version;
    std::vector<uint8_t> salt;
    std::vector<uint8_t> encrypted_data_key;
    std::vector<uint8_t> data_key_nonce;
};

// Main vault class
class Vault {
public:
    Vault();
    ~Vault();
    
    // Initialization
    bool create(const std::string& path, const std::string& passphrase);
    bool open(const std::string& path);
    
    // Lock/Unlock
    bool unlock(const std::string& passphrase);
    void lock();
    
    // Entry management
    std::string add_entry(const Entry& entry);
    bool update_entry(const std::string& id, const Entry& entry);
    bool delete_entry(const std::string& id);
    Entry get_entry(const std::string& id) const;
    std::vector<Entry> list_entries() const;
    
    // Search
    std::vector<Entry> search(const std::string& query) const;
    std::vector<Entry> search_by_tag(const std::string& tag) const;
    
    // Key rotation
    bool rekey(const std::vector<uint8_t>& new_data_key);
    
    // State
    VaultState state() const { return state_; }
    bool is_unlocked() const { return state_ == VaultState::UNLOCKED; }
    
    // Persistence
    bool save();
    bool load();
    
private:
    std::string path_;
    VaultState state_;
    VaultHeader header_;
    
    // Decrypted data (in secure memory)
    std::unique_ptr<memory::SecureBuffer> master_key_;
    std::unique_ptr<memory::SecureBuffer> data_key_;
    std::map<std::string, Entry> entries_;
    
    // Serialization
    std::vector<uint8_t> serialize_entries() const;
    void deserialize_entries(const std::vector<uint8_t>& data);
    
    // Helper
    std::string generate_id() const;
    void ensure_unlocked() const;
};

} // namespace vault
} // namespace passdoq

#endif // PASSDOQ_VAULT_HPP
