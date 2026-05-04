#ifndef PASSDOQ_DELIVERY_HPP
#define PASSDOQ_DELIVERY_HPP

#include <string>
#include <vector>
#include <memory>
#include <cstdint>

namespace passdoq {
namespace delivery {

// Delivery method interface
class DeliveryMethod {
public:
    virtual ~DeliveryMethod() = default;
    
    virtual bool deliver(const std::vector<uint8_t>& secret) = 0;
    virtual std::string name() const = 0;
};

// Clipboard delivery
class ClipboardDelivery : public DeliveryMethod {
public:
    ClipboardDelivery();
    ~ClipboardDelivery() override;
    
    bool deliver(const std::vector<uint8_t>& secret) override;
    std::string name() const override { return "clipboard"; }
    
private:
    void clear_clipboard();
};

// File delivery
class FileDelivery : public DeliveryMethod {
public:
    FileDelivery(const std::string& path);
    ~FileDelivery() override = default;
    
    bool deliver(const std::vector<uint8_t>& secret) override;
    std::string name() const override { return "file"; }
    
private:
    std::string path_;
};

// Stdout delivery
class StdoutDelivery : public DeliveryMethod {
public:
    StdoutDelivery() = default;
    ~StdoutDelivery() override = default;
    
    bool deliver(const std::vector<uint8_t>& secret) override;
    std::string name() const override { return "stdout"; }
};

// Secure memory delivery (returns reference)
class MemoryDelivery : public DeliveryMethod {
public:
    MemoryDelivery();
    ~MemoryDelivery() override;
    
    bool deliver(const std::vector<uint8_t>& secret) override;
    std::string name() const override { return "memory"; }
    
    const uint8_t* data() const { return data_; }
    size_t size() const { return size_; }
    
private:
    uint8_t* data_;
    size_t size_;
};

// Delivery manager
class DeliveryManager {
public:
    DeliveryManager();
    ~DeliveryManager();
    
    // Register delivery method
    void register_method(std::unique_ptr<DeliveryMethod> method);
    
    // Deliver secret
    bool deliver(const std::string& method_name, const std::vector<uint8_t>& secret);
    
    // List available methods
    std::vector<std::string> list_methods() const;
    
private:
    std::vector<std::unique_ptr<DeliveryMethod>> methods_;
};

} // namespace delivery
} // namespace passdoq

#endif // PASSDOQ_DELIVERY_HPP
