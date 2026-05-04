#include <catch2/catch_test_macros.hpp>
#include "../src/daemon.hpp"
#include "../src/crypto.hpp"

using namespace passdoq::daemon;

TEST_CASE("Daemon configuration", "[daemon]") {
    DaemonConfig config;
    
    REQUIRE(config.pid_file == "/var/run/passdoqd.pid");
    REQUIRE(config.socket_path == "/var/run/passdoqd.sock");
    REQUIRE(config.daemonize == true);
}

TEST_CASE("Daemon basic operations", "[daemon]") {
    REQUIRE(passdoq::crypto::init());
    
    DaemonConfig config;
    config.daemonize = false;  // Don't actually daemonize in tests
    config.pid_file = "/tmp/test_passdoqd.pid";
    
    Daemon daemon(config);
    
    SECTION("Initial state") {
        REQUIRE_FALSE(daemon.is_running());
    }
    
    SECTION("Service registry access") {
        auto& services = daemon.services();
        REQUIRE(services.list_services().empty());
    }
}
