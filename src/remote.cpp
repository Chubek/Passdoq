#include "remote.hpp"
#include <iostream>

namespace passdoq {
namespace remote {

// RpcServer implementation
RpcServer::RpcServer(uint16_t port)
    : server_(std::make_unique<rpc::server>(port))
    , vault_(nullptr)
    , port_(port)
    , running_(false) {
    setup_methods();
}

RpcServer::~RpcServer() {
    stop();
}

void RpcServer::bind_vault(vault::Vault* vault) {
    vault_ = vault;
}

bool RpcServer::start() {
    if (running_) {
        return false;
    }
    
    running_ = true;
    std::cout << "RPC server started on port " << port_ << "\n";
    return true;
}

bool RpcServer::stop() {
    if (!running_) {
        return false;
    }
    
    server_->stop();
    running_ = false;
    std::cout << "RPC server stopped\n";
    return true;
}

bool RpcServer::is_running() const {
    return running_;
}

void RpcServer::run() {
    if (!running_) {
        start();
    }
    
    server_->run();
}

void RpcServer::setup_methods() {
    // Unlock vault
    server_->bind("unlock", [this](const std::string& passphrase) -> bool {
        if (!vault_) return false;
        return vault_->unlock(passphrase);
    });
    
    // Lock vault
    server_->bind("lock", [this]() {
        if (vault_) {
            vault_->lock();
        }
    });
    
    // Add entry
    server_->bind("add_entry", [this](const std::string& name,
                                      const std::string& username,
                                      const std::string& secret) -> std::string {
        if (!vault_ || !vault_->is_unlocked()) {
            return "";
        }
        
        vault::Entry entry;
        entry.name = name;
        entry.username = username;
        entry.secret = std::vector<uint8_t>(secret.begin(), secret.end());
        
        std::string id = vault_->add_entry(entry);
        vault_->save();
        
        return id;
    });
    
    // Get entry
    server_->bind("get_entry", [this](const std::string& id) -> std::tuple<std::string, std::string, std::string> {
        if (!vault_ || !vault_->is_unlocked()) {
            return std::make_tuple("", "", "");
        }
        
        try {
            auto entry = vault_->get_entry(id);
            std::string secret(entry.secret.begin(), entry.secret.end());
            return std::make_tuple(entry.name, entry.username, secret);
        } catch (const std::exception&) {
            return std::make_tuple("", "", "");
        }
    });
    
    // List entries
    server_->bind("list_entries", [this]() -> std::vector<std::string> {
        if (!vault_ || !vault_->is_unlocked()) {
            return {};
        }
        
        auto entries = vault_->list_entries();
        std::vector<std::string> ids;
        
        for (const auto& entry : entries) {
            ids.push_back(entry.id);
        }
        
        return ids;
    });
    
    // Delete entry
    server_->bind("delete_entry", [this](const std::string& id) -> bool {
        if (!vault_ || !vault_->is_unlocked()) {
            return false;
        }
        
        bool result = vault_->delete_entry(id);
        if (result) {
            vault_->save();
        }
        
        return result;
    });
}

// RpcClient implementation
RpcClient::RpcClient(const std::string& host, uint16_t port)
    : client_(std::make_unique<rpc::client>(host, port))
    , host_(host)
    , port_(port)
    , connected_(false) {
}

RpcClient::~RpcClient() {
    disconnect();
}

bool RpcClient::connect() {
    try {
        // rpclib client connects automatically on first call
        connected_ = true;
        std::cout << "Connected to RPC server at " << host_ << ":" << port_ << "\n";
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Connection failed: " << e.what() << "\n";
        return false;
    }
}

void RpcClient::disconnect() {
    connected_ = false;
}

bool RpcClient::is_connected() const {
    return connected_;
}

bool RpcClient::unlock(const std::string& passphrase) {
    try {
        return client_->call("unlock", passphrase).as<bool>();
    } catch (const std::exception& e) {
        std::cerr << "RPC error: " << e.what() << "\n";
        return false;
    }
}

void RpcClient::lock() {
    try {
        client_->call("lock");
    } catch (const std::exception& e) {
        std::cerr << "RPC error: " << e.what() << "\n";
    }
}

std::string RpcClient::add_entry(const std::string& name,
                                  const std::string& username,
                                  const std::string& secret) {
    try {
        return client_->call("add_entry", name, username, secret).as<std::string>();
    } catch (const std::exception& e) {
        std::cerr << "RPC error: " << e.what() << "\n";
        return "";
    }
}

bool RpcClient::get_entry(const std::string& id,
                          std::string& name,
                          std::string& username,
                          std::string& secret) {
    try {
        auto result = client_->call("get_entry", id).as<std::tuple<std::string, std::string, std::string>>();
        
        name = std::get<0>(result);
        username = std::get<1>(result);
        secret = std::get<2>(result);
        
        return !name.empty();
    } catch (const std::exception& e) {
        std::cerr << "RPC error: " << e.what() << "\n";
        return false;
    }
}

std::vector<std::string> RpcClient::list_entries() {
    try {
        return client_->call("list_entries").as<std::vector<std::string>>();
    } catch (const std::exception& e) {
        std::cerr << "RPC error: " << e.what() << "\n";
        return {};
    }
}

bool RpcClient::delete_entry(const std::string& id) {
    try {
        return client_->call("delete_entry", id).as<bool>();
    } catch (const std::exception& e) {
        std::cerr << "RPC error: " << e.what() << "\n";
        return false;
    }
}

} // namespace remote
} // namespace passdoq
