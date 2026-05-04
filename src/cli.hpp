#ifndef PASSDOQ_CLI_HPP
#define PASSDOQ_CLI_HPP

#include "vault.hpp"
#include "runcom.hpp"
#include <string>
#include <memory>
#include <CLI/CLI.hpp>

namespace passdoq {
namespace cli {

// CLI Application
class Application {
public:
    Application();
    ~Application();
    
    // Run the application
    int run(int argc, char** argv);
    
private:
    std::unique_ptr<CLI::App> app_;
    std::unique_ptr<vault::Vault> vault_;
    std::unique_ptr<runcom::Config> config_;
    
    std::string vault_path_;
    bool verbose_;
    
    // Setup commands
    void setup_commands();
    
    // Command handlers
    int cmd_init();
    int cmd_add();
    int cmd_get();
    int cmd_list();
    int cmd_search();
    int cmd_update();
    int cmd_delete();
    int cmd_lock();
    int cmd_unlock();
    int cmd_config();
    
    // Helper functions
    bool ensure_vault_open();
    bool ensure_vault_unlocked();
    std::string prompt_password(const std::string& prompt = "Password: ");
    std::string prompt_input(const std::string& prompt);
    void print_entry(const vault::Entry& entry, bool show_secret = false);
    std::vector<uint8_t> read_secret();
};

} // namespace cli
} // namespace passdoq

#endif // PASSDOQ_CLI_HPP
