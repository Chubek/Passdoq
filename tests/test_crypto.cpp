#include <catch2/catch_test_macros.hpp>
#include "../src/crypto.hpp"
#include <string>

using namespace passdoq::crypto;

TEST_CASE("Crypto initialization", "[crypto]") {
    REQUIRE(init());
}

TEST_CASE("Master key derivation", "[crypto]") {
    REQUIRE(init());
    
    std::string passphrase = "test_password_123";
    
    SECTION("Derive key with random salt") {
        auto result = derive_master_key(passphrase);
        
        REQUIRE(result.key.size() == MASTER_KEY_SIZE);
        REQUIRE(result.salt.size() == SALT_SIZE);
    }
    
    SECTION("Derive key with provided salt") {
        auto first = derive_master_key(passphrase);
        auto second = derive_master_key(passphrase, first.salt);
        
        REQUIRE(first.key == second.key);
        REQUIRE(first.salt == second.salt);
    }
    
    SECTION("Different passphrases produce different keys") {
        auto key1 = derive_master_key("password1");
        auto key2 = derive_master_key("password2");
        
        REQUIRE(key1.key != key2.key);
    }
}

TEST_CASE("Data key generation", "[crypto]") {
    REQUIRE(init());
    
    auto key1 = generate_data_key();
    auto key2 = generate_data_key();
    
    REQUIRE(key1.size() == DATA_KEY_SIZE);
    REQUIRE(key2.size() == DATA_KEY_SIZE);
    REQUIRE(key1 != key2);
}

TEST_CASE("Random bytes generation", "[crypto]") {
    REQUIRE(init());
    
    auto bytes1 = random_bytes(32);
    auto bytes2 = random_bytes(32);
    
    REQUIRE(bytes1.size() == 32);
    REQUIRE(bytes2.size() == 32);
    REQUIRE(bytes1 != bytes2);
}

TEST_CASE("Encryption and decryption", "[crypto]") {
    REQUIRE(init());
    
    auto key = generate_data_key();
    std::string plaintext_str = "This is a secret message!";
    std::vector<uint8_t> plaintext(plaintext_str.begin(), plaintext_str.end());
    
    SECTION("Encrypt and decrypt successfully") {
        auto encrypted = encrypt(plaintext, key);
        
        REQUIRE(!encrypted.ciphertext.empty());
        REQUIRE(encrypted.nonce.size() == NONCE_SIZE);
        REQUIRE(!encrypted.mac.empty());
        
        auto decrypted = decrypt(encrypted, key);
        
        REQUIRE(decrypted == plaintext);
    }
    
    SECTION("Decryption with wrong key fails") {
        auto encrypted = encrypt(plaintext, key);
        auto wrong_key = generate_data_key();
        
        REQUIRE_THROWS(decrypt(encrypted, wrong_key));
    }
    
    SECTION("Decryption with tampered ciphertext fails") {
        auto encrypted = encrypt(plaintext, key);
        encrypted.ciphertext[0] ^= 1; // Flip a bit
        
        REQUIRE_THROWS(decrypt(encrypted, key));
    }
}

TEST_CASE("Secure memory zeroing", "[crypto]") {
    REQUIRE(init());
    
    std::vector<uint8_t> data = {1, 2, 3, 4, 5};
    secure_zero(data.data(), data.size());
    
    for (auto byte : data) {
        REQUIRE(byte == 0);
    }
}
