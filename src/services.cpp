#include "services.hpp"
#include <algorithm>
#include <iostream>

namespace passdoq {
namespace services {

// ServiceRegistry implementation
ServiceRegistry::ServiceRegistry() {}

ServiceRegistry::~ServiceRegistry() {
    stop_all();
}

bool ServiceRegistry::register_service(std::unique_ptr<Service> service) {
    if (!service) {
        return false;
    }
    
    std::string name = service->name();
    
    if (services_.find(name) != services_.end()) {
        std::cerr << "Service already registered: " << name << "\n";
        return false;
    }
    
    services_[name] = std::move(service);
    return true;
}

bool ServiceRegistry::unregister_service(const std::string& name) {
    auto it = services_.find(name);
    if (it == services_.end()) {
        return false;
    }
    
    // Stop service if running
    if (it->second->status() == ServiceStatus::RUNNING) {
        it->second->stop();
    }
    
    services_.erase(it);
    return true;
}

bool ServiceRegistry::start_service(const std::string& name) {
    auto it = services_.find(name);
    if (it == services_.end()) {
        return false;
    }
    
    return it->second->start();
}

bool ServiceRegistry::stop_service(const std::string& name) {
    auto it = services_.find(name);
    if (it == services_.end()) {
        return false;
    }
    
    return it->second->stop();
}

bool ServiceRegistry::restart_service(const std::string& name) {
    if (!stop_service(name)) {
        return false;
    }
    
    return start_service(name);
}

Service* ServiceRegistry::get_service(const std::string& name) const {
    auto it = services_.find(name);
    if (it == services_.end()) {
        return nullptr;
    }
    
    return it->second.get();
}

std::vector<std::string> ServiceRegistry::list_services() const {
    std::vector<std::string> names;
    for (const auto& pair : services_) {
        names.push_back(pair.first);
    }
    return names;
}

ServiceStatus ServiceRegistry::get_status(const std::string& name) const {
    auto service = get_service(name);
    if (!service) {
        return ServiceStatus::STOPPED;
    }
    
    return service->status();
}

bool ServiceRegistry::start_all() {
    bool all_started = true;
    
    for (auto& pair : services_) {
        if (!pair.second->start()) {
            std::cerr << "Failed to start service: " << pair.first << "\n";
            all_started = false;
        }
    }
    
    return all_started;
}

bool ServiceRegistry::stop_all() {
    bool all_stopped = true;
    
    for (auto& pair : services_) {
        if (!pair.second->stop()) {
            std::cerr << "Failed to stop service: " << pair.first << "\n";
            all_stopped = false;
        }
    }
    
    return all_stopped;
}

// ServiceManager implementation
ServiceManager& ServiceManager::instance() {
    static ServiceManager instance;
    return instance;
}

void ServiceManager::register_factory(const std::string& type, ServiceFactory factory) {
    factories_[type] = factory;
}

std::unique_ptr<Service> ServiceManager::create_service(const std::string& type) {
    auto it = factories_.find(type);
    if (it == factories_.end()) {
        return nullptr;
    }
    
    return it->second();
}

} // namespace services
} // namespace passdoq
