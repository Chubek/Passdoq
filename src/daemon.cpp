#include "daemon.hpp"
#include <fstream>
#include <iostream>
#include <csignal>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

namespace passdoq {
namespace daemon {

Daemon* Daemon::instance_ = nullptr;

Daemon::Daemon(const DaemonConfig& config)
    : config_(config)
    , vault_(nullptr)
    , running_(false) {
    instance_ = this;
}

Daemon::~Daemon() {
    stop();
    instance_ = nullptr;
}

bool Daemon::start() {
    if (running_) {
        return false;
    }
    
    // Daemonize if requested
    if (config_.daemonize) {
        if (!daemonize()) {
            return false;
        }
    }
    
    // Write PID file
    if (!write_pid_file()) {
        return false;
    }
    
    // Setup signal handlers
    std::signal(SIGTERM, signal_handler);
    std::signal(SIGINT, signal_handler);
    
    // Start services
    if (!services_.start_all()) {
        std::cerr << "Failed to start all services\n";
        return false;
    }
    
    running_ = true;
    
    // Start worker thread
    worker_thread_ = std::make_unique<std::thread>(&Daemon::run, this);
    
    std::cout << "Daemon started\n";
    return true;
}

bool Daemon::stop() {
    if (!running_) {
        return false;
    }
    
    running_ = false;
    
    // Wait for worker thread
    if (worker_thread_ && worker_thread_->joinable()) {
        worker_thread_->join();
    }
    
    // Stop services
    services_.stop_all();
    
    // Remove PID file
    remove_pid_file();
    
    std::cout << "Daemon stopped\n";
    return true;
}

bool Daemon::is_running() const {
    return running_;
}

bool Daemon::daemonize() {
    // Fork first time
    pid_t pid = fork();
    
    if (pid < 0) {
        std::cerr << "First fork failed\n";
        return false;
    }
    
    if (pid > 0) {
        // Parent exits
        exit(0);
    }
    
    // Create new session
    if (setsid() < 0) {
        std::cerr << "setsid failed\n";
        return false;
    }
    
    // Fork second time
    pid = fork();
    
    if (pid < 0) {
        std::cerr << "Second fork failed\n";
        return false;
    }
    
    if (pid > 0) {
        // Parent exits
        exit(0);
    }
    
    // Change working directory
    if (chdir("/") < 0) {
        std::cerr << "chdir failed\n";
        return false;
    }
    
    // Close standard file descriptors
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
    
    // Redirect to /dev/null or log file
    int fd = open("/dev/null", O_RDWR);
    if (fd >= 0) {
        dup2(fd, STDIN_FILENO);
        dup2(fd, STDOUT_FILENO);
        dup2(fd, STDERR_FILENO);
        if (fd > STDERR_FILENO) {
            close(fd);
        }
    }
    
    return true;
}

bool Daemon::write_pid_file() {
    std::ofstream file(config_.pid_file);
    if (!file) {
        std::cerr << "Failed to write PID file: " << config_.pid_file << "\n";
        return false;
    }
    
    file << getpid() << "\n";
    return true;
}

bool Daemon::remove_pid_file() {
    return unlink(config_.pid_file.c_str()) == 0;
}

void Daemon::run() {
    while (running_) {
        // Main daemon loop
        // Process IPC messages, handle requests, etc.
        
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

void Daemon::signal_handler(int signum) {
    if (instance_) {
        std::cout << "Received signal " << signum << ", stopping daemon\n";
        instance_->stop();
    }
}

} // namespace daemon
} // namespace passdoq
