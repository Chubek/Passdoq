#ifndef PASSDOQ_CRYPTO_HPP
#define PASSDOQ_CRYPTO_HPP

#include <cstdint>
#include <vector>
#include <string>
#include <memory>

namespace passdoq {
namespace crypto {

// Constants
constexpr size_t MASTER_KEY_SIZE = 32;
constexpr size_t DATA_KEY_SIZE = 32;
constexpr size_t SALT_SIZE = 16;
constexpr size_t NONCE_SIZE = 24;
constexpr size_t MAC_SIZE = 16;

// Argon2id parameters
constexpr uint64_t ARGON2_OPSLIMIT = 3;
constexpr size_t ARGON2_MEMLIMIT = 67108864; // 64 MB

// Result types
struct DerivedKey {
    std::vector<uint8_t> key;
    std::vector<uint8_t> salt;
};

struct EncryptedData {
    std::vector<uint8_t> ciphertext;
    std::vector<uint8_t> nonce;
    std::vector<uint8_t> mac;
};

// Initialize libsodium
bool init();

// Master key derivation using Argon2id
DerivedKey derive_master_key(const std::string& passphrase, 
                              const std::vector<uint8_t>& salt = {});

// Generate random data key
std::vector<uint8_t> generate_data_key();

// Generate random bytes
std::vector<uint8_t> random_bytes(size_t size);

// Encrypt data using secretbox (XSalsa20-Poly1305)
EncryptedData encrypt(const std::vector<uint8_t>& plaintext,
                      const std::vector<uint8_t>& key);

// Decrypt data
std::vector<uint8_t> decrypt(const EncryptedData& encrypted,
                              const std::vector<uint8_t>& key);

// Secure memory zeroing
void secure_zero(void* ptr, size_t size);

} // namespace crypto
} // namespace passdoq

#endif // PASSDOQ_CRYPTO_HPP
