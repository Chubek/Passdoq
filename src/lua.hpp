#ifndef PASSDOQ_LUA_HPP
#define PASSDOQ_LUA_HPP

#include "vault.hpp"
#include <string>
#include <memory>
#include <sol/sol.hpp>

namespace passdoq {
namespace lua {

// Lua addon info
struct AddonInfo {
    std::string name;
    std::string author;
    std::string maintainer;
    std::string version;
};

// Lua engine
class LuaEngine {
public:
    LuaEngine();
    ~LuaEngine();
    
    // Initialize Lua state
    bool init();
    
    // Load and execute scripts
    bool load_file(const std::string& path);
    bool load_string(const std::string& code);
    
    // Execute function
    bool call_function(const std::string& name);
    
    // Bind vault
    void bind_vault(vault::Vault* vault);
    
    // Get addon info
    std::vector<AddonInfo> list_addons() const;
    
    // Event system
    void trigger_event(const std::string& event_name);
    void trigger_event(const std::string& event_name, const vault::Entry& entry);
    
private:
    std::unique_ptr<sol::state> lua_;
    vault::Vault* vault_;
    std::vector<AddonInfo> addons_;
    
    // Setup bindings
    void setup_bindings();
    void bind_crypto();
    void bind_vault_api();
    void bind_addon_api();
};

} // namespace lua
} // namespace passdoq

#endif // PASSDOQ_LUA_HPP
