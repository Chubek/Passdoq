#include "delivery.hpp"
#include "crypto.hpp"
#include <fstream>
#include <iostream>
#include <cstring>

namespace passdoq {
namespace delivery {

// ClipboardDelivery implementation
ClipboardDelivery::ClipboardDelivery() {}

ClipboardDelivery::~ClipboardDelivery() {
    clear_clipboard();
}

bool ClipboardDelivery::deliver(const std::vector<uint8_t>& secret) {
    // In production, would use libclipboard
    // For now, just print a message
    std::cout << "[Clipboard] Secret copied to clipboard\n";
    return true;
}

void ClipboardDelivery::clear_clipboard() {
    std::cout << "[Clipboard] Clipboard cleared\n";
}

// FileDelivery implementation
FileDelivery::FileDelivery(const std::string& path) : path_(path) {}

bool FileDelivery::deliver(const std::vector<uint8_t>& secret) {
    std::ofstream file(path_, std::ios::binary);
    if (!file) {
        std::cerr << "Failed to open file: " << path_ << "\n";
        return false;
    }
    
    file.write(reinterpret_cast<const char*>(secret.data()), secret.size());
    
    if (!file) {
        std::cerr << "Failed to write to file: " << path_ << "\n";
        return false;
    }
    
    std::cout << "[File] Secret written to: " << path_ << "\n";
    return true;
}

// StdoutDelivery implementation
bool StdoutDelivery::deliver(const std::vector<uint8_t>& secret) {
    std::cout.write(reinterpret_cast<const char*>(secret.data()), secret.size());
    std::cout << "\n";
    return true;
}

// MemoryDelivery implementation
MemoryDelivery::MemoryDelivery() : data_(nullptr), size_(0) {}

MemoryDelivery::~MemoryDelivery() {
    if (data_) {
        crypto::secure_zero(data_, size_);
        delete[] data_;
    }
}

bool MemoryDelivery::deliver(const std::vector<uint8_t>& secret) {
    // Clear old data
    if (data_) {
        crypto::secure_zero(data_, size_);
        delete[] data_;
    }
    
    // Allocate new buffer
    size_ = secret.size();
    data_ = new uint8_t[size_];
    
    // Copy secret
    std::memcpy(data_, secret.data(), size_);
    
    std::cout << "[Memory] Secret stored in secure memory\n";
    return true;
}

// DeliveryManager implementation
DeliveryManager::DeliveryManager() {}

DeliveryManager::~DeliveryManager() = default;

void DeliveryManager::register_method(std::unique_ptr<DeliveryMethod> method) {
    methods_.push_back(std::move(method));
}

bool DeliveryManager::deliver(const std::string& method_name, const std::vector<uint8_t>& secret) {
    for (auto& method : methods_) {
        if (method->name() == method_name) {
            return method->deliver(secret);
        }
    }
    
    std::cerr << "Delivery method not found: " << method_name << "\n";
    return false;
}

std::vector<std::string> DeliveryManager::list_methods() const {
    std::vector<std::string> names;
    for (const auto& method : methods_) {
        names.push_back(method->name());
    }
    return names;
}

} // namespace delivery
} // namespace passdoq
