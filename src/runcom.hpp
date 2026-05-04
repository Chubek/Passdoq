#ifndef PASSDOQ_RUNCOM_HPP
#define PASSDOQ_RUNCOM_HPP

#include <string>
#include <map>
#include <vector>
#include <memory>

namespace passdoq {
namespace runcom {

// Configuration value types
enum class ValueType {
    STRING,
    INTEGER,
    BOOLEAN,
    LIST
};

// Configuration value
struct Value {
    ValueType type;
    std::string string_value;
    int64_t int_value;
    bool bool_value;
    std::vector<std::string> list_value;
    
    Value() : type(ValueType::STRING), int_value(0), bool_value(false) {}
    
    static Value from_string(const std::string& s);
    static Value from_int(int64_t i);
    static Value from_bool(bool b);
    static Value from_list(const std::vector<std::string>& l);
};

// Configuration store
class Config {
public:
    Config();
    
    // Load configuration from file
    bool load(const std::string& path);
    
    // Load from default locations
    bool load_defaults();
    
    // Get values
    std::string get_string(const std::string& key, const std::string& default_val = "") const;
    int64_t get_int(const std::string& key, int64_t default_val = 0) const;
    bool get_bool(const std::string& key, bool default_val = false) const;
    std::vector<std::string> get_list(const std::string& key) const;
    
    // Set values
    void set(const std::string& key, const Value& value);
    void set_string(const std::string& key, const std::string& value);
    void set_int(const std::string& key, int64_t value);
    void set_bool(const std::string& key, bool value);
    void set_list(const std::string& key, const std::vector<std::string>& value);
    
    // Check existence
    bool has(const std::string& key) const;
    
    // Get all keys
    std::vector<std::string> keys() const;
    
private:
    std::map<std::string, Value> values_;
    std::vector<std::string> loaded_files_;
    
    // Parse PRL (Passdoq Runcom Language)
    bool parse_prl(const std::string& content, const std::string& base_path);
    
    // Preprocess with ucpp
    std::string preprocess(const std::string& content, const std::string& base_path);
    
    // Check for circular includes
    bool is_circular_include(const std::string& path) const;
};

// Get default config paths
std::vector<std::string> get_default_paths();

} // namespace runcom
} // namespace passdoq

#endif // PASSDOQ_RUNCOM_HPP
