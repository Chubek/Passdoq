#ifndef PASSDOQ_REMOTE_HPP
#define PASSDOQ_REMOTE_HPP

#include "vault.hpp"
#include <string>
#include <memory>
#include <rpc/server.h>
#include <rpc/client.h>

namespace passdoq {
namespace remote {

// RPC server
class RpcServer {
public:
    RpcServer(uint16_t port);
    ~RpcServer();
    
    // Bind vault
    void bind_vault(vault::Vault* vault);
    
    // Server control
    bool start();
    bool stop();
    bool is_running() const;
    
    // Run server (blocking)
    void run();
    
private:
    std::unique_ptr<rpc::server> server_;
    vault::Vault* vault_;
    uint16_t port_;
    bool running_;
    
    // Setup RPC methods
    void setup_methods();
};

// RPC client
class RpcClient {
public:
    RpcClient(const std::string& host, uint16_t port);
    ~RpcClient();
    
    // Connection
    bool connect();
    void disconnect();
    bool is_connected() const;
    
    // Vault operations
    bool unlock(const std::string& passphrase);
    void lock();
    
    // Entry operations
    std::string add_entry(const std::string& name, 
                          const std::string& username,
                          const std::string& secret);
    
    bool get_entry(const std::string& id,
                   std::string& name,
                   std::string& username,
                   std::string& secret);
    
    std::vector<std::string> list_entries();
    
    bool delete_entry(const std::string& id);
    
private:
    std::unique_ptr<rpc::client> client_;
    std::string host_;
    uint16_t port_;
    bool connected_;
};

} // namespace remote
} // namespace passdoq

#endif // PASSDOQ_REMOTE_HPP
