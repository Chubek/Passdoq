#include "lua.hpp"
#include "crypto.hpp"
#include <iostream>

namespace passdoq {
namespace lua {

LuaEngine::LuaEngine() : vault_(nullptr) {}

LuaEngine::~LuaEngine() = default;

bool LuaEngine::init() {
    try {
        lua_ = std::make_unique<sol::state>();
        lua_->open_libraries(sol::lib::base, sol::lib::string, sol::lib::table);
        
        setup_bindings();
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "Lua initialization failed: " << e.what() << "\n";
        return false;
    }
}

bool LuaEngine::load_file(const std::string& path) {
    if (!lua_) {
        return false;
    }
    
    try {
        lua_->script_file(path);
        return true;
    } catch (const sol::error& e) {
        std::cerr << "Lua script error: " << e.what() << "\n";
        return false;
    }
}

bool LuaEngine::load_string(const std::string& code) {
    if (!lua_) {
        return false;
    }
    
    try {
        lua_->script(code);
        return true;
    } catch (const sol::error& e) {
        std::cerr << "Lua script error: " << e.what() << "\n";
        return false;
    }
}

bool LuaEngine::call_function(const std::string& name) {
    if (!lua_) {
        return false;
    }
    
    try {
        sol::function func = (*lua_)[name];
        if (func.valid()) {
            func();
            return true;
        }
        return false;
    } catch (const sol::error& e) {
        std::cerr << "Lua function call error: " << e.what() << "\n";
        return false;
    }
}

void LuaEngine::bind_vault(vault::Vault* vault) {
    vault_ = vault;
}

std::vector<AddonInfo> LuaEngine::list_addons() const {
    return addons_;
}

void LuaEngine::trigger_event(const std::string& event_name) {
    if (!lua_) return;
    
    try {
        sol::table events = (*lua_)["_passdoq_events"];
        if (events.valid()) {
            sol::table handlers = events[event_name];
            if (handlers.valid()) {
                for (const auto& pair : handlers) {
                    sol::function handler = pair.second;
                    if (handler.valid()) {
                        handler();
                    }
                }
            }
        }
    } catch (const sol::error& e) {
        std::cerr << "Event trigger error: " << e.what() << "\n";
    }
}

void LuaEngine::trigger_event(const std::string& event_name, const vault::Entry& entry) {
    if (!lua_) return;
    
    try {
        sol::table events = (*lua_)["_passdoq_events"];
        if (events.valid()) {
            sol::table handlers = events[event_name];
            if (handlers.valid()) {
                // Create Lua table for entry
                sol::table entry_table = lua_->create_table();
                entry_table["id"] = entry.id;
                entry_table["name"] = entry.name;
                entry_table["username"] = entry.username;
                
                for (const auto& pair : handlers) {
                    sol::function handler = pair.second;
                    if (handler.valid()) {
                        handler(entry_table);
                    }
                }
            }
        }
    } catch (const sol::error& e) {
        std::cerr << "Event trigger error: " << e.what() << "\n";
    }
}

void LuaEngine::setup_bindings() {
    bind_crypto();
    bind_vault_api();
    bind_addon_api();
}

void LuaEngine::bind_crypto() {
    auto passdoq_table = lua_->create_named_table("lpassdoq");
    
    passdoq_table["random_bytes"] = [](int size) -> std::string {
        auto bytes = crypto::random_bytes(size);
        return std::string(bytes.begin(), bytes.end());
    };
}

void LuaEngine::bind_vault_api() {
    auto passdoq_table = (*lua_)["lpassdoq"];
    
    passdoq_table["get_entry"] = [this](const std::string& id) -> sol::table {
        if (!vault_ || !vault_->is_unlocked()) {
            return lua_->create_table();
        }
        
        try {
            auto entry = vault_->get_entry(id);
            
            sol::table result = lua_->create_table();
            result["id"] = entry.id;
            result["name"] = entry.name;
            result["username"] = entry.username;
            
            // Convert secret to string
            std::string secret(entry.secret.begin(), entry.secret.end());
            result["value"] = secret;
            
            return result;
            
        } catch (const std::exception&) {
            return lua_->create_table();
        }
    };
    
    passdoq_table["list_entries"] = [this]() -> sol::table {
        sol::table result = lua_->create_table();
        
        if (!vault_ || !vault_->is_unlocked()) {
            return result;
        }
        
        try {
            auto entries = vault_->list_entries();
            
            for (size_t i = 0; i < entries.size(); ++i) {
                sol::table entry_table = lua_->create_table();
                entry_table["id"] = entries[i].id;
                entry_table["name"] = entries[i].name;
                entry_table["username"] = entries[i].username;
                
                result[i + 1] = entry_table;
            }
            
        } catch (const std::exception&) {
            // Return empty table
        }
        
        return result;
    };
}

void LuaEngine::bind_addon_api() {
    auto passdoq_table = (*lua_)["lpassdoq"];
    
    // Initialize addon
    passdoq_table["init_addon"] = [this](sol::table info) -> sol::table {
        AddonInfo addon;
        addon.name = info.get_or<std::string>("name", "Unknown");
        addon.author = info.get_or<std::string>("author", "Unknown");
        addon.maintainer = info.get_or<std::string>("maintainer", "Unknown");
        addon.version = info.get_or<std::string>("version", "1.0.0");
        
        addons_.push_back(addon);
        
        // Return addon API
        sol::table api = lua_->create_table();
        
        // Event registration
        api["on"] = [this](const std::string& event_name, sol::function handler) {
            sol::table events = (*lua_)["_passdoq_events"];
            if (!events.valid()) {
                events = lua_->create_named_table("_passdoq_events");
            }
            
            sol::table handlers = events[event_name];
            if (!handlers.valid()) {
                handlers = lua_->create_table();
                events[event_name] = handlers;
            }
            
            handlers[handlers.size() + 1] = handler;
        };
        
        // Print function
        api["print"] = [](const std::string& message) {
            std::cout << "[Passdoq] " << message << "\n";
        };
        
        // Clipboard operations (stub)
        api["clipboard_set"] = [](const std::string& value) {
            // Would use libclipboard in production
            std::cout << "[Clipboard] Set value\n";
        };
        
        api["clipboard_clear"] = []() {
            std::cout << "[Clipboard] Cleared\n";
        };
        
        // Deferred execution (stub)
        api["defer"] = [](int seconds, sol::function callback) {
            // Would use timer in production
            std::cout << "[Defer] Scheduled callback for " << seconds << " seconds\n";
        };
        
        return api;
    };
}

} // namespace lua
} // namespace passdoq
