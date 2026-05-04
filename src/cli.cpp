#include "cli.hpp"
#include "crypto.hpp"
#include <iostream>
#include <iomanip>
#include <termios.h>
#include <unistd.h>

namespace passdoq {
namespace cli {

Application::Application() 
    : app_(std::make_unique<CLI::App>("Passdoq - CLI Password Manager"))
    , vault_(std::make_unique<vault::Vault>())
    , config_(std::make_unique<runcom::Config>())
    , verbose_(false) {
    
    // Global options
    app_->add_option("-v,--vault", vault_path_, "Vault file path")
        ->default_val("~/.passdoq.vault");
    app_->add_flag("--verbose", verbose_, "Verbose output");
    
    setup_commands();
}

Application::~Application() = default;

int Application::run(int argc, char** argv) {
    // Initialize crypto
    if (!crypto::init()) {
        std::cerr << "Failed to initialize crypto library\n";
        return 1;
    }
    
    // Load config
    config_->load_defaults();
    
    // Override vault path from config if not specified
    if (vault_path_.empty() || vault_path_ == "~/.passdoq.vault") {
        vault_path_ = config_->get_string("vault.path", "~/.passdoq.vault");
    }
    
    // Expand ~ in path
    if (vault_path_[0] == '~') {
        const char* home = std::getenv("HOME");
        if (home) {
            vault_path_ = std::string(home) + vault_path_.substr(1);
        }
    }
    
    try {
        app_->parse(argc, argv);
    } catch (const CLI::ParseError& e) {
        return app_->exit(e);
    }
    
    return 0;
}

void Application::setup_commands() {
    // init command
    auto init_cmd = app_->add_subcommand("init", "Initialize a new vault");
    init_cmd->callback([this]() { return cmd_init(); });
    
    // add command
    auto add_cmd = app_->add_subcommand("add", "Add a new entry");
    add_cmd->callback([this]() { return cmd_add(); });
    
    // get command
    auto get_cmd = app_->add_subcommand("get", "Get an entry");
    get_cmd->callback([this]() { return cmd_get(); });
    
    // list command
    auto list_cmd = app_->add_subcommand("list", "List all entries");
    list_cmd->callback([this]() { return cmd_list(); });
    
    // search command
    auto search_cmd = app_->add_subcommand("search", "Search entries");
    search_cmd->callback([this]() { return cmd_search(); });
    
    // update command
    auto update_cmd = app_->add_subcommand("update", "Update an entry");
    update_cmd->callback([this]() { return cmd_update(); });
    
    // delete command
    auto delete_cmd = app_->add_subcommand("delete", "Delete an entry");
    delete_cmd->callback([this]() { return cmd_delete(); });
    
    // lock command
    auto lock_cmd = app_->add_subcommand("lock", "Lock the vault");
    lock_cmd->callback([this]() { return cmd_lock(); });
    
    // unlock command
    auto unlock_cmd = app_->add_subcommand("unlock", "Unlock the vault");
    unlock_cmd->callback([this]() { return cmd_unlock(); });
    
    // config command
    auto config_cmd = app_->add_subcommand("config", "Manage configuration");
    config_cmd->callback([this]() { return cmd_config(); });
}

int Application::cmd_init() {
    std::cout << "Initializing new vault at: " << vault_path_ << "\n";
    
    std::string password = prompt_password("Enter master password: ");
    std::string confirm = prompt_password("Confirm master password: ");
    
    if (password != confirm) {
        std::cerr << "Passwords do not match\n";
        return 1;
    }
    
    if (!vault_->create(vault_path_, password)) {
        std::cerr << "Failed to create vault\n";
        return 1;
    }
    
    std::cout << "Vault created successfully\n";
    return 0;
}

int Application::cmd_add() {
    if (!ensure_vault_unlocked()) {
        return 1;
    }
    
    vault::Entry entry;
    
    entry.name = prompt_input("Name: ");
    entry.username = prompt_input("Username: ");
    entry.secret = read_secret();
    
    std::string tags_str = prompt_input("Tags (comma-separated): ");
    if (!tags_str.empty()) {
        std::istringstream iss(tags_str);
        std::string tag;
        while (std::getline(iss, tag, ',')) {
            // Trim whitespace
            tag.erase(0, tag.find_first_not_of(" \t"));
            tag.erase(tag.find_last_not_of(" \t") + 1);
            if (!tag.empty()) {
                entry.tags.push_back(tag);
            }
        }
    }
    
    std::string id = vault_->add_entry(entry);
    
    if (!vault_->save()) {
        std::cerr << "Failed to save vault\n";
        return 1;
    }
    
    std::cout << "Entry added with ID: " << id << "\n";
    return 0;
}

int Application::cmd_get() {
    if (!ensure_vault_unlocked()) {
        return 1;
    }
    
    std::string id = prompt_input("Entry ID: ");
    
    try {
        auto entry = vault_->get_entry(id);
        print_entry(entry, true);
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
}

int Application::cmd_list() {
    if (!ensure_vault_unlocked()) {
        return 1;
    }
    
    auto entries = vault_->list_entries();
    
    if (entries.empty()) {
        std::cout << "No entries found\n";
        return 0;
    }
    
    std::cout << "\nEntries:\n";
    std::cout << std::string(80, '-') << "\n";
    
    for (const auto& entry : entries) {
        print_entry(entry, false);
        std::cout << std::string(80, '-') << "\n";
    }
    
    return 0;
}

int Application::cmd_search() {
    if (!ensure_vault_unlocked()) {
        return 1;
    }
    
    std::string query = prompt_input("Search query: ");
    
    auto entries = vault_->search(query);
    
    if (entries.empty()) {
        std::cout << "No entries found\n";
        return 0;
    }
    
    std::cout << "\nFound " << entries.size() << " entries:\n";
    std::cout << std::string(80, '-') << "\n";
    
    for (const auto& entry : entries) {
        print_entry(entry, false);
        std::cout << std::string(80, '-') << "\n";
    }
    
    return 0;
}

int Application::cmd_update() {
    if (!ensure_vault_unlocked()) {
        return 1;
    }
    
    std::string id = prompt_input("Entry ID: ");
    
    try {
        auto entry = vault_->get_entry(id);
        
        std::cout << "Current entry:\n";
        print_entry(entry, false);
        
        std::cout << "\nEnter new values (leave empty to keep current):\n";
        
        std::string name = prompt_input("Name [" + entry.name + "]: ");
        if (!name.empty()) entry.name = name;
        
        std::string username = prompt_input("Username [" + entry.username + "]: ");
        if (!username.empty()) entry.username = username;
        
        std::string update_secret = prompt_input("Update secret? (y/n): ");
        if (update_secret == "y" || update_secret == "yes") {
            entry.secret = read_secret();
        }
        
        if (!vault_->update_entry(id, entry)) {
            std::cerr << "Failed to update entry\n";
            return 1;
        }
        
        if (!vault_->save()) {
            std::cerr << "Failed to save vault\n";
            return 1;
        }
        
        std::cout << "Entry updated successfully\n";
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
}

int Application::cmd_delete() {
    if (!ensure_vault_unlocked()) {
        return 1;
    }
    
    std::string id = prompt_input("Entry ID: ");
    
    std::string confirm = prompt_input("Are you sure? (yes/no): ");
    if (confirm != "yes") {
        std::cout << "Cancelled\n";
        return 0;
    }
    
    if (!vault_->delete_entry(id)) {
        std::cerr << "Failed to delete entry\n";
        return 1;
    }
    
    if (!vault_->save()) {
        std::cerr << "Failed to save vault\n";
        return 1;
    }
    
    std::cout << "Entry deleted successfully\n";
    return 0;
}

int Application::cmd_lock() {
    vault_->lock();
    std::cout << "Vault locked\n";
    return 0;
}

int Application::cmd_unlock() {
    if (!ensure_vault_open()) {
        return 1;
    }
    
    std::string password = prompt_password();
    
    if (!vault_->unlock(password)) {
        std::cerr << "Failed to unlock vault (incorrect password?)\n";
        return 1;
    }
    
    std::cout << "Vault unlocked\n";
    return 0;
}

int Application::cmd_config() {
    std::cout << "Configuration:\n";
    
    for (const auto& key : config_->keys()) {
        std::cout << "  " << key << " = " << config_->get_string(key) << "\n";
    }
    
    return 0;
}

bool Application::ensure_vault_open() {
    if (vault_->state() == vault::VaultState::UNINITIALIZED) {
        if (!vault_->open(vault_path_)) {
            std::cerr << "Failed to open vault. Use 'init' to create a new vault.\n";
            return false;
        }
    }
    return true;
}

bool Application::ensure_vault_unlocked() {
    if (!ensure_vault_open()) {
        return false;
    }
    
    if (!vault_->is_unlocked()) {
        std::string password = prompt_password();
        if (!vault_->unlock(password)) {
            std::cerr << "Failed to unlock vault\n";
            return false;
        }
    }
    
    return true;
}

std::string Application::prompt_password(const std::string& prompt) {
    std::cout << prompt;
    std::cout.flush();
    
    // Disable echo
    termios old_term, new_term;
    tcgetattr(STDIN_FILENO, &old_term);
    new_term = old_term;
    new_term.c_lflag &= ~ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &new_term);
    
    std::string password;
    std::getline(std::cin, password);
    
    // Restore echo
    tcsetattr(STDIN_FILENO, TCSANOW, &old_term);
    
    std::cout << "\n";
    return password;
}

std::string Application::prompt_input(const std::string& prompt) {
    std::cout << prompt;
    std::cout.flush();
    
    std::string input;
    std::getline(std::cin, input);
    
    return input;
}

void Application::print_entry(const vault::Entry& entry, bool show_secret) {
    std::cout << "ID: " << entry.id << "\n";
    std::cout << "Name: " << entry.name << "\n";
    std::cout << "Username: " << entry.username << "\n";
    
    if (show_secret) {
        std::cout << "Secret: ";
        for (auto byte : entry.secret) {
            std::cout << static_cast<char>(byte);
        }
        std::cout << "\n";
    }
    
    if (!entry.tags.empty()) {
        std::cout << "Tags: ";
        for (size_t i = 0; i < entry.tags.size(); ++i) {
            std::cout << entry.tags[i];
            if (i < entry.tags.size() - 1) std::cout << ", ";
        }
        std::cout << "\n";
    }
    
    if (!entry.metadata.empty()) {
        std::cout << "Metadata:\n";
        for (const auto& pair : entry.metadata) {
            std::cout << "  " << pair.first << ": " << pair.second << "\n";
        }
    }
}

std::vector<uint8_t> Application::read_secret() {
    std::string secret = prompt_password("Secret: ");
    return std::vector<uint8_t>(secret.begin(), secret.end());
}

} // namespace cli
} // namespace passdoq
