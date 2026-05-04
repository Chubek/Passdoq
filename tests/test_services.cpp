#include <catch2/catch_test_macros.hpp>
#include "../src/services.hpp"

using namespace passdoq::services;

// Mock service for testing
class MockService : public Service {
public:
    MockService(const std::string& name) 
        : name_(name), status_(ServiceStatus::STOPPED) {}
    
    bool start() override {
        status_ = ServiceStatus::RUNNING;
        return true;
    }
    
    bool stop() override {
        status_ = ServiceStatus::STOPPED;
        return true;
    }
    
    ServiceStatus status() const override {
        return status_;
    }
    
    std::string name() const override {
        return name_;
    }
    
    std::string description() const override {
        return "Mock service for testing";
    }
    
private:
    std::string name_;
    ServiceStatus status_;
};

TEST_CASE("Service registry operations", "[services]") {
    ServiceRegistry registry;
    
    SECTION("Register service") {
        auto service = std::make_unique<MockService>("test_service");
        REQUIRE(registry.register_service(std::move(service)));
        
        auto names = registry.list_services();
        REQUIRE(names.size() == 1);
        REQUIRE(names[0] == "test_service");
    }
    
    SECTION("Start and stop service") {
        auto service = std::make_unique<MockService>("test_service");
        registry.register_service(std::move(service));
        
        REQUIRE(registry.start_service("test_service"));
        REQUIRE(registry.get_status("test_service") == ServiceStatus::RUNNING);
        
        REQUIRE(registry.stop_service("test_service"));
        REQUIRE(registry.get_status("test_service") == ServiceStatus::STOPPED);
    }
    
    SECTION("Restart service") {
        auto service = std::make_unique<MockService>("test_service");
        registry.register_service(std::move(service));
        
        registry.start_service("test_service");
        REQUIRE(registry.restart_service("test_service"));
        REQUIRE(registry.get_status("test_service") == ServiceStatus::RUNNING);
    }
    
    SECTION("Unregister service") {
        auto service = std::make_unique<MockService>("test_service");
        registry.register_service(std::move(service));
        
        REQUIRE(registry.unregister_service("test_service"));
        REQUIRE(registry.list_services().empty());
    }
}

TEST_CASE("Service manager", "[services]") {
    auto& manager = ServiceManager::instance();
    
    SECTION("Register and create service") {
        manager.register_factory("mock", []() -> std::unique_ptr<Service> {
            return std::make_unique<MockService>("factory_service");
        });
        
        auto service = manager.create_service("mock");
        REQUIRE(service != nullptr);
        REQUIRE(service->name() == "factory_service");
    }
    
    SECTION("Create non-existent service") {
        auto service = manager.create_service("nonexistent");
        REQUIRE(service == nullptr);
    }
}

TEST_CASE("Multiple services", "[services]") {
    ServiceRegistry registry;
    
    registry.register_service(std::make_unique<MockService>("service1"));
    registry.register_service(std::make_unique<MockService>("service2"));
    registry.register_service(std::make_unique<MockService>("service3"));
    
    SECTION("Start all services") {
        REQUIRE(registry.start_all());
        
        REQUIRE(registry.get_status("service1") == ServiceStatus::RUNNING);
        REQUIRE(registry.get_status("service2") == ServiceStatus::RUNNING);
        REQUIRE(registry.get_status("service3") == ServiceStatus::RUNNING);
    }
    
    SECTION("Stop all services") {
        registry.start_all();
        REQUIRE(registry.stop_all());
        
        REQUIRE(registry.get_status("service1") == ServiceStatus::STOPPED);
        REQUIRE(registry.get_status("service2") == ServiceStatus::STOPPED);
        REQUIRE(registry.get_status("service3") == ServiceStatus::STOPPED);
    }
}
