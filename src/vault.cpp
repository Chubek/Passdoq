#include "vault.hpp"
#include <fstream>
#include <sstream>
#include <iomanip>
#include <random>
#include <stdexcept>
#include <msgpack.hpp>

namespace passdoq {
namespace vault {

// Serialization helpers for Entry
namespace {
    std::string time_to_string(const std::chrono::system_clock::time_point& tp) {
        auto time = std::chrono::system_clock::to_time_t(tp);
        std::ostringstream oss;
        oss << time;
        return oss.str();
    }
    
    std::chrono::system_clock::time_point string_to_time(const std::string& str) {
        std::istringstream iss(str);
        std::time_t time;
        iss >> time;
        return std::chrono::system_clock::from_time_t(time);
    }
}

Vault::Vault() : state_(VaultState::UNINITIALIZED) {}

Vault::~Vault() {
    lock();
}

bool Vault::create(const std::string& path, const std::string& passphrase) {
    if (state_ != VaultState::UNINITIALIZED) {
        return false;
    }
    
    path_ = path;
    
    // Derive master key
    auto derived = crypto::derive_master_key(passphrase);
    master_key_ = std::make_unique<memory::SecureBuffer>(crypto::MASTER_KEY_SIZE);
    std::copy(derived.key.begin(), derived.key.end(), master_key_->data());
    
    // Generate data key
    auto data_key = crypto::generate_data_key();
    data_key_ = std::make_unique<memory::SecureBuffer>(crypto::DATA_KEY_SIZE);
    std::copy(data_key.begin(), data_key.end(), data_key_->data());
    
    // Encrypt data key with master key
    std::vector<uint8_t> data_key_vec(data_key_->data(), data_key_->data() + data_key_->size());
    auto encrypted_dk = crypto::encrypt(data_key_vec, derived.key);
    
    // Setup header
    header_.version = 1;
    header_.salt = derived.salt;
    header_.encrypted_data_key = encrypted_dk.ciphertext;
    header_.data_key_nonce = encrypted_dk.nonce;
    
    state_ = VaultState::UNLOCKED;
    
    // Save empty vault
    return save();
}

bool Vault::open(const std::string& path) {
    if (state_ != VaultState::UNINITIALIZED) {
        return false;
    }
    
    path_ = path;
    
    if (!load()) {
        return false;
    }
    
    state_ = VaultState::LOCKED;
    return true;
}

bool Vault::unlock(const std::string& passphrase) {
    if (state_ != VaultState::LOCKED) {
        return false;
    }
    
    // Derive master key from passphrase
    auto derived = crypto::derive_master_key(passphrase, header_.salt);
    
    // Decrypt data key
    crypto::EncryptedData encrypted_dk;
    encrypted_dk.ciphertext = header_.encrypted_data_key;
    encrypted_dk.nonce = header_.data_key_nonce;
    
    try {
        auto data_key = crypto::decrypt(encrypted_dk, derived.key);
        
        // Store keys in secure memory
        master_key_ = std::make_unique<memory::SecureBuffer>(crypto::MASTER_KEY_SIZE);
        std::copy(derived.key.begin(), derived.key.end(), master_key_->data());
        
        data_key_ = std::make_unique<memory::SecureBuffer>(crypto::DATA_KEY_SIZE);
        std::copy(data_key.begin(), data_key.end(), data_key_->data());
        
        state_ = VaultState::UNLOCKED;
        
        // Load and decrypt entries
        return load();
        
    } catch (const std::exception&) {
        return false;
    }
}

void Vault::lock() {
    if (state_ == VaultState::UNLOCKED) {
        // Zero sensitive data
        if (master_key_) {
            master_key_->zero();
            master_key_.reset();
        }
        if (data_key_) {
            data_key_->zero();
            data_key_.reset();
        }
        
        // Clear entries
        entries_.clear();
        
        state_ = VaultState::LOCKED;
    }
}

std::string Vault::add_entry(const Entry& entry) {
    ensure_unlocked();
    
    Entry new_entry = entry;
    new_entry.id = generate_id();
    new_entry.created = std::chrono::system_clock::now();
    new_entry.modified = new_entry.created;
    
    entries_[new_entry.id] = new_entry;
    
    return new_entry.id;
}

bool Vault::update_entry(const std::string& id, const Entry& entry) {
    ensure_unlocked();
    
    auto it = entries_.find(id);
    if (it == entries_.end()) {
        return false;
    }
    
    Entry updated = entry;
    updated.id = id;
    updated.created = it->second.created;
    updated.modified = std::chrono::system_clock::now();
    
    entries_[id] = updated;
    return true;
}

bool Vault::delete_entry(const std::string& id) {
    ensure_unlocked();
    return entries_.erase(id) > 0;
}

Entry Vault::get_entry(const std::string& id) const {
    ensure_unlocked();
    
    auto it = entries_.find(id);
    if (it == entries_.end()) {
        throw std::runtime_error("Entry not found");
    }
    
    return it->second;
}

std::vector<Entry> Vault::list_entries() const {
    ensure_unlocked();
    
    std::vector<Entry> result;
    for (const auto& pair : entries_) {
        result.push_back(pair.second);
    }
    return result;
}

std::vector<Entry> Vault::search(const std::string& query) const {
    ensure_unlocked();
    
    std::vector<Entry> result;
    for (const auto& pair : entries_) {
        const auto& entry = pair.second;
        if (entry.name.find(query) != std::string::npos ||
            entry.username.find(query) != std::string::npos) {
            result.push_back(entry);
        }
    }
    return result;
}

std::vector<Entry> Vault::search_by_tag(const std::string& tag) const {
    ensure_unlocked();
    
    std::vector<Entry> result;
    for (const auto& pair : entries_) {
        const auto& entry = pair.second;
        if (std::find(entry.tags.begin(), entry.tags.end(), tag) != entry.tags.end()) {
            result.push_back(entry);
        }
    }
    return result;
}

bool Vault::rekey(const std::vector<uint8_t>& new_data_key) {
    ensure_unlocked();
    
    if (new_data_key.size() != crypto::DATA_KEY_SIZE) {
        return false;
    }
    
    // Update data key
    std::copy(new_data_key.begin(), new_data_key.end(), data_key_->data());
    
    // Re-encrypt data key with master key
    std::vector<uint8_t> master_key_vec(master_key_->data(), 
                                        master_key_->data() + master_key_->size());
    auto encrypted_dk = crypto::encrypt(new_data_key, master_key_vec);
    
    header_.encrypted_data_key = encrypted_dk.ciphertext;
    header_.data_key_nonce = encrypted_dk.nonce;
    
    return save();
}

bool Vault::save() {
    if (state_ != VaultState::UNLOCKED) {
        return false;
    }
    
    try {
        // Serialize entries
        auto serialized = serialize_entries();
        
        // Encrypt entries
        std::vector<uint8_t> data_key_vec(data_key_->data(), 
                                          data_key_->data() + data_key_->size());
        auto encrypted = crypto::encrypt(serialized, data_key_vec);
        
        // Pack everything with msgpack
        msgpack::sbuffer buffer;
        msgpack::packer<msgpack::sbuffer> packer(buffer);
        
        packer.pack_map(5);
        
        packer.pack("version");
        packer.pack(header_.version);
        
        packer.pack("salt");
        packer.pack(header_.salt);
        
        packer.pack("encrypted_data_key");
        packer.pack(header_.encrypted_data_key);
        
        packer.pack("data_key_nonce");
        packer.pack(header_.data_key_nonce);
        
        packer.pack("encrypted_entries");
        packer.pack_map(2);
        packer.pack("ciphertext");
        packer.pack(encrypted.ciphertext);
        packer.pack("nonce");
        packer.pack(encrypted.nonce);
        
        // Write to file
        std::ofstream file(path_, std::ios::binary);
        if (!file) {
            return false;
        }
        
        file.write(buffer.data(), buffer.size());
        return file.good();
        
    } catch (const std::exception&) {
        return false;
    }
}

bool Vault::load() {
    try {
        // Read file
        std::ifstream file(path_, std::ios::binary);
        if (!file) {
            return false;
        }
        
        std::vector<uint8_t> data((std::istreambuf_iterator<char>(file)),
                                   std::istreambuf_iterator<char>());
        
        // Unpack with msgpack
        msgpack::object_handle oh = msgpack::unpack(reinterpret_cast<const char*>(data.data()), 
                                                      data.size());
        msgpack::object obj = oh.get();
        
        auto map = obj.as<std::map<std::string, msgpack::object>>();
        
        header_.version = map["version"].as<uint32_t>();
        header_.salt = map["salt"].as<std::vector<uint8_t>>();
        header_.encrypted_data_key = map["encrypted_data_key"].as<std::vector<uint8_t>>();
        header_.data_key_nonce = map["data_key_nonce"].as<std::vector<uint8_t>>();
        
        // If unlocked, decrypt entries
        if (state_ == VaultState::UNLOCKED && data_key_) {
            auto enc_entries = map["encrypted_entries"].as<std::map<std::string, msgpack::object>>();
            
            crypto::EncryptedData encrypted;
            encrypted.ciphertext = enc_entries["ciphertext"].as<std::vector<uint8_t>>();
            encrypted.nonce = enc_entries["nonce"].as<std::vector<uint8_t>>();
            
            std::vector<uint8_t> data_key_vec(data_key_->data(), 
                                              data_key_->data() + data_key_->size());
            auto decrypted = crypto::decrypt(encrypted, data_key_vec);
            
            deserialize_entries(decrypted);
        }
        
        return true;
        
    } catch (const std::exception&) {
        return false;
    }
}

std::vector<uint8_t> Vault::serialize_entries() const {
    msgpack::sbuffer buffer;
    msgpack::packer<msgpack::sbuffer> packer(buffer);
    
    packer.pack_array(entries_.size());
    
    for (const auto& pair : entries_) {
        const auto& entry = pair.second;
        
        packer.pack_map(8);
        
        packer.pack("id");
        packer.pack(entry.id);
        
        packer.pack("name");
        packer.pack(entry.name);
        
        packer.pack("username");
        packer.pack(entry.username);
        
        packer.pack("secret");
        packer.pack(entry.secret);
        
        packer.pack("metadata");
        packer.pack(entry.metadata);
        
        packer.pack("tags");
        packer.pack(entry.tags);
        
        packer.pack("created");
        packer.pack(time_to_string(entry.created));
        
        packer.pack("modified");
        packer.pack(time_to_string(entry.modified));
    }
    
    return std::vector<uint8_t>(buffer.data(), buffer.data() + buffer.size());
}

void Vault::deserialize_entries(const std::vector<uint8_t>& data) {
    entries_.clear();
    
    msgpack::object_handle oh = msgpack::unpack(reinterpret_cast<const char*>(data.data()), 
                                                  data.size());
    msgpack::object obj = oh.get();
    
    auto array = obj.as<std::vector<msgpack::object>>();
    
    for (const auto& item : array) {
        auto map = item.as<std::map<std::string, msgpack::object>>();
        
        Entry entry;
        entry.id = map["id"].as<std::string>();
        entry.name = map["name"].as<std::string>();
        entry.username = map["username"].as<std::string>();
        entry.secret = map["secret"].as<std::vector<uint8_t>>();
        entry.metadata = map["metadata"].as<std::map<std::string, std::string>>();
        entry.tags = map["tags"].as<std::vector<std::string>>();
        entry.created = string_to_time(map["created"].as<std::string>());
        entry.modified = string_to_time(map["modified"].as<std::string>());
        
        entries_[entry.id] = entry;
    }
}

std::string Vault::generate_id() const {
    auto bytes = crypto::random_bytes(16);
    std::ostringstream oss;
    for (auto b : bytes) {
        oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(b);
    }
    return oss.str();
}

void Vault::ensure_unlocked() const {
    if (state_ != VaultState::UNLOCKED) {
        throw std::runtime_error("Vault is locked");
    }
}

} // namespace vault
} // namespace passdoq
