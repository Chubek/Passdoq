#ifndef PASSDOQ_DAEMON_HPP
#define PASSDOQ_DAEMON_HPP

#include "services.hpp"
#include "vault.hpp"
#include <string>
#include <memory>
#include <atomic>
#include <thread>

namespace passdoq {
namespace daemon {

// Daemon configuration
struct DaemonConfig {
    std::string pid_file;
    std::string socket_path;
    std::string log_file;
    bool daemonize;
    
    DaemonConfig()
        : pid_file("/var/run/passdoqd.pid")
        , socket_path("/var/run/passdoqd.sock")
        , log_file("/var/log/passdoqd.log")
        , daemonize(true) {}
};

// Daemon class
class Daemon {
public:
    Daemon(const DaemonConfig& config);
    ~Daemon();
    
    // Lifecycle
    bool start();
    bool stop();
    bool is_running() const;
    
    // Service management
    services::ServiceRegistry& services() { return services_; }
    
    // Vault access
    void set_vault(vault::Vault* vault) { vault_ = vault; }
    vault::Vault* vault() const { return vault_; }
    
private:
    DaemonConfig config_;
    services::ServiceRegistry services_;
    vault::Vault* vault_;
    
    std::atomic<bool> running_;
    std::unique_ptr<std::thread> worker_thread_;
    
    // Daemon operations
    bool daemonize();
    bool write_pid_file();
    bool remove_pid_file();
    
    // Main loop
    void run();
    
    // Signal handling
    static void signal_handler(int signum);
    static Daemon* instance_;
};

} // namespace daemon
} // namespace passdoq

#endif // PASSDOQ_DAEMON_HPP
