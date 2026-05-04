#include "crypto.hpp"
#include <sodium.h>
#include <stdexcept>
#include <cstring>

namespace passdoq {
namespace crypto {

bool init() {
    return sodium_init() >= 0;
}

DerivedKey derive_master_key(const std::string& passphrase, 
                              const std::vector<uint8_t>& salt) {
    DerivedKey result;
    result.key.resize(MASTER_KEY_SIZE);
    
    // Generate or use provided salt
    if (salt.empty()) {
        result.salt.resize(SALT_SIZE);
        randombytes_buf(result.salt.data(), SALT_SIZE);
    } else {
        result.salt = salt;
    }
    
    // Derive key using Argon2id
    if (crypto_pwhash(result.key.data(), MASTER_KEY_SIZE,
                      passphrase.c_str(), passphrase.length(),
                      result.salt.data(),
                      ARGON2_OPSLIMIT,
                      ARGON2_MEMLIMIT,
                      crypto_pwhash_ALG_ARGON2ID13) != 0) {
        throw std::runtime_error("Argon2id key derivation failed");
    }
    
    return result;
}

std::vector<uint8_t> generate_data_key() {
    std::vector<uint8_t> key(DATA_KEY_SIZE);
    randombytes_buf(key.data(), DATA_KEY_SIZE);
    return key;
}

std::vector<uint8_t> random_bytes(size_t size) {
    std::vector<uint8_t> bytes(size);
    randombytes_buf(bytes.data(), size);
    return bytes;
}

EncryptedData encrypt(const std::vector<uint8_t>& plaintext,
                      const std::vector<uint8_t>& key) {
    if (key.size() != crypto_secretbox_KEYBYTES) {
        throw std::invalid_argument("Invalid key size");
    }
    
    EncryptedData result;
    result.nonce.resize(crypto_secretbox_NONCEBYTES);
    result.ciphertext.resize(plaintext.size() + crypto_secretbox_MACBYTES);
    
    // Generate random nonce
    randombytes_buf(result.nonce.data(), crypto_secretbox_NONCEBYTES);
    
    // Encrypt using XSalsa20-Poly1305
    if (crypto_secretbox_easy(result.ciphertext.data(),
                              plaintext.data(), plaintext.size(),
                              result.nonce.data(),
                              key.data()) != 0) {
        throw std::runtime_error("Encryption failed");
    }
    
    // Extract MAC (first MACBYTES of ciphertext)
    result.mac.assign(result.ciphertext.begin(), 
                      result.ciphertext.begin() + crypto_secretbox_MACBYTES);
    
    return result;
}

std::vector<uint8_t> decrypt(const EncryptedData& encrypted,
                              const std::vector<uint8_t>& key) {
    if (key.size() != crypto_secretbox_KEYBYTES) {
        throw std::invalid_argument("Invalid key size");
    }
    
    if (encrypted.ciphertext.size() < crypto_secretbox_MACBYTES) {
        throw std::invalid_argument("Invalid ciphertext size");
    }
    
    std::vector<uint8_t> plaintext(encrypted.ciphertext.size() - crypto_secretbox_MACBYTES);
    
    // Decrypt using XSalsa20-Poly1305
    if (crypto_secretbox_open_easy(plaintext.data(),
                                    encrypted.ciphertext.data(),
                                    encrypted.ciphertext.size(),
                                    encrypted.nonce.data(),
                                    key.data()) != 0) {
        throw std::runtime_error("Decryption failed (authentication error)");
    }
    
    return plaintext;
}

void secure_zero(void* ptr, size_t size) {
    sodium_memzero(ptr, size);
}

} // namespace crypto
} // namespace passdoq
