#include "modules.hpp"
#include "crypto.hpp"
#include <ltdl.h>
#include <iostream>

namespace passdoq {
namespace modules {

bool ModuleManager::ltdl_initialized_ = false;

// ModuleHandle implementation
ModuleHandle::ModuleHandle(void* handle, passdoq_module_t* module)
    : handle_(handle), module_(module) {
}

ModuleHandle::~ModuleHandle() {
    if (handle_) {
        if (module_ && module_->deinit) {
            module_->deinit(nullptr);
        }
        lt_dlclose(static_cast<lt_dlhandle>(handle_));
    }
}

ModuleHandle::ModuleHandle(ModuleHandle&& other) noexcept
    : handle_(other.handle_), module_(other.module_) {
    other.handle_ = nullptr;
    other.module_ = nullptr;
}

ModuleHandle& ModuleHandle::operator=(ModuleHandle&& other) noexcept {
    if (this != &other) {
        if (handle_) {
            if (module_ && module_->deinit) {
                module_->deinit(nullptr);
            }
            lt_dlclose(static_cast<lt_dlhandle>(handle_));
        }
        handle_ = other.handle_;
        module_ = other.module_;
        other.handle_ = nullptr;
        other.module_ = nullptr;
    }
    return *this;
}

const std::string& ModuleHandle::name() const {
    static std::string empty;
    return module_ ? std::string(module_->name) : empty;
}

// ModuleManager implementation
ModuleManager::ModuleManager() {
    init_ltdl();
}

ModuleManager::~ModuleManager() {
    unload_all();
    cleanup_ltdl();
}

bool ModuleManager::init_ltdl() {
    if (!ltdl_initialized_) {
        if (lt_dlinit() != 0) {
            return false;
        }
        ltdl_initialized_ = true;
    }
    return true;
}

void ModuleManager::cleanup_ltdl() {
    if (ltdl_initialized_) {
        lt_dlexit();
        ltdl_initialized_ = false;
    }
}

bool ModuleManager::load_module(const std::string& path) {
    if (!ltdl_initialized_) {
        return false;
    }
    
    // Open module
    lt_dlhandle handle = lt_dlopen(path.c_str());
    if (!handle) {
        std::cerr << "Failed to load module: " << lt_dlerror() << "\n";
        return false;
    }
    
    // Get init function
    auto init_fn = reinterpret_cast<passdoq_module_init_fn>(
        lt_dlsym(handle, "passdoq_module_init"));
    
    if (!init_fn) {
        std::cerr << "Module missing passdoq_module_init function\n";
        lt_dlclose(handle);
        return false;
    }
    
    // Initialize module
    passdoq_module_t* module = init_fn();
    if (!module) {
        std::cerr << "Module initialization failed\n";
        lt_dlclose(handle);
        return false;
    }
    
    // Check if already loaded
    if (is_loaded(module->name)) {
        std::cerr << "Module already loaded: " << module->name << "\n";
        lt_dlclose(handle);
        return false;
    }
    
    // Call module init hook
    if (module->init) {
        if (module->init(nullptr) != 0) {
            std::cerr << "Module init hook failed\n";
            lt_dlclose(handle);
            return false;
        }
    }
    
    // Store module
    modules_[module->name] = std::make_unique<ModuleHandle>(handle, module);
    
    std::cout << "Loaded module: " << module->name << " v" << module->version << "\n";
    return true;
}

bool ModuleManager::unload_module(const std::string& name) {
    auto it = modules_.find(name);
    if (it == modules_.end()) {
        return false;
    }
    
    modules_.erase(it);
    return true;
}

void ModuleManager::unload_all() {
    modules_.clear();
}

bool ModuleManager::is_loaded(const std::string& name) const {
    return modules_.find(name) != modules_.end();
}

std::vector<std::string> ModuleManager::list_modules() const {
    std::vector<std::string> names;
    for (const auto& pair : modules_) {
        names.push_back(pair.first);
    }
    return names;
}

passdoq_module_t* ModuleManager::get_module(const std::string& name) const {
    auto it = modules_.find(name);
    if (it == modules_.end()) {
        return nullptr;
    }
    return it->second->module();
}

void ModuleManager::on_vault_unlock(passdoq_ctx_t* ctx) {
    for (const auto& pair : modules_) {
        auto module = pair.second->module();
        if (module && module->on_vault_unlock) {
            module->on_vault_unlock(ctx);
        }
    }
}

void ModuleManager::on_vault_lock(passdoq_ctx_t* ctx) {
    for (const auto& pair : modules_) {
        auto module = pair.second->module();
        if (module && module->on_vault_lock) {
            module->on_vault_lock(ctx);
        }
    }
}

void ModuleManager::on_config(passdoq_ctx_t* ctx, const std::string& key, const std::string& value) {
    for (const auto& pair : modules_) {
        auto module = pair.second->module();
        if (module && module->on_config) {
            module->on_config(ctx, key.c_str(), value.c_str());
        }
    }
}

} // namespace modules
} // namespace passdoq

// C API implementations for modules
extern "C" {

void passdoq_log(passdoq_ctx_t* ctx, passdoq_log_level_t level, const char* message) {
    const char* level_str = "INFO";
    switch (level) {
        case PASSDOQ_LOG_DEBUG: level_str = "DEBUG"; break;
        case PASSDOQ_LOG_INFO: level_str = "INFO"; break;
        case PASSDOQ_LOG_WARNING: level_str = "WARNING"; break;
        case PASSDOQ_LOG_ERROR: level_str = "ERROR"; break;
    }
    std::cout << "[" << level_str << "] " << message << "\n";
}

int passdoq_crypto_random(passdoq_ctx_t* ctx, uint8_t* buf, size_t size) {
    auto bytes = passdoq::crypto::random_bytes(size);
    std::copy(bytes.begin(), bytes.end(), buf);
    return 0;
}

int passdoq_vault_rekey(passdoq_ctx_t* ctx, const uint8_t* new_key, size_t key_size) {
    if (!ctx) return -1;
    
    auto module_ctx = reinterpret_cast<passdoq::modules::ModuleContext*>(ctx);
    if (!module_ctx->vault) return -1;
    
    std::vector<uint8_t> key(new_key, new_key + key_size);
    return module_ctx->vault->rekey(key) ? 0 : -1;
}

void passdoq_secure_memzero(void* ptr, size_t size) {
    passdoq::crypto::secure_zero(ptr, size);
}

} // extern "C"
