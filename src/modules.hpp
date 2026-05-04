#ifndef PASSDOQ_MODULES_HPP
#define PASSDOQ_MODULES_HPP

#include "vault.hpp"
#include "../include/passdoq-module.h"
#include <string>
#include <vector>
#include <memory>
#include <map>

namespace passdoq {
namespace modules {

// Module handle wrapper
class ModuleHandle {
public:
    ModuleHandle(void* handle, passdoq_module_t* module);
    ~ModuleHandle();
    
    // No copy
    ModuleHandle(const ModuleHandle&) = delete;
    ModuleHandle& operator=(const ModuleHandle&) = delete;
    
    // Move allowed
    ModuleHandle(ModuleHandle&& other) noexcept;
    ModuleHandle& operator=(ModuleHandle&& other) noexcept;
    
    passdoq_module_t* module() const { return module_; }
    const std::string& name() const;
    
private:
    void* handle_;
    passdoq_module_t* module_;
};

// Module manager
class ModuleManager {
public:
    ModuleManager();
    ~ModuleManager();
    
    // Load/unload modules
    bool load_module(const std::string& path);
    bool unload_module(const std::string& name);
    void unload_all();
    
    // Query modules
    bool is_loaded(const std::string& name) const;
    std::vector<std::string> list_modules() const;
    passdoq_module_t* get_module(const std::string& name) const;
    
    // Hook invocation
    void on_vault_unlock(passdoq_ctx_t* ctx);
    void on_vault_lock(passdoq_ctx_t* ctx);
    void on_config(passdoq_ctx_t* ctx, const std::string& key, const std::string& value);
    
private:
    std::map<std::string, std::unique_ptr<ModuleHandle>> modules_;
    
    // Initialize libltdl
    static bool init_ltdl();
    static void cleanup_ltdl();
    static bool ltdl_initialized_;
};

// Context for module API
struct ModuleContext {
    vault::Vault* vault;
    void* user_data;
};

} // namespace modules
} // namespace passdoq

#endif // PASSDOQ_MODULES_HPP
