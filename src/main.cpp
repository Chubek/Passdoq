#include "cli.hpp"
#include <iostream>

int main(int argc, char** argv) {
    try {
        passdoq::cli::Application app;
        return app.run(argc, argv);
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << "\n";
        return 1;
    }
}
