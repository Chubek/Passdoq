#ifndef PASSDOQ_SERVICES_HPP
#define PASSDOQ_SERVICES_HPP

#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <map>

namespace passdoq {
namespace services {

// Service status
enum class ServiceStatus {
    STOPPED,
    STARTING,
    RUNNING,
    STOPPING,
    FAILED
};

// Service interface
class Service {
public:
    virtual ~Service() = default;
    
    virtual bool start() = 0;
    virtual bool stop() = 0;
    virtual ServiceStatus status() const = 0;
    
    virtual std::string name() const = 0;
    virtual std::string description() const = 0;
};

// Service registry
class ServiceRegistry {
public:
    ServiceRegistry();
    ~ServiceRegistry();
    
    // Register/unregister services
    bool register_service(std::unique_ptr<Service> service);
    bool unregister_service(const std::string& name);
    
    // Service control
    bool start_service(const std::string& name);
    bool stop_service(const std::string& name);
    bool restart_service(const std::string& name);
    
    // Query services
    Service* get_service(const std::string& name) const;
    std::vector<std::string> list_services() const;
    ServiceStatus get_status(const std::string& name) const;
    
    // Lifecycle
    bool start_all();
    bool stop_all();
    
private:
    std::map<std::string, std::unique_ptr<Service>> services_;
};

// Service factory
using ServiceFactory = std::function<std::unique_ptr<Service>()>;

class ServiceManager {
public:
    static ServiceManager& instance();
    
    // Register factory
    void register_factory(const std::string& type, ServiceFactory factory);
    
    // Create service
    std::unique_ptr<Service> create_service(const std::string& type);
    
private:
    ServiceManager() = default;
    std::map<std::string, ServiceFactory> factories_;
};

} // namespace services
} // namespace passdoq

#endif // PASSDOQ_SERVICES_HPP
