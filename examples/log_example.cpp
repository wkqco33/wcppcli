#include "wcppcli/wlog.hpp"
#include <iostream>

using namespace wcppcli;

int main() {
    WLog::info("Starting application...");
    
    WLog::debug("Checking for configuration files...");
    WLog::success("Configuration loaded successfully.");
    
    WLog::warn("Port 80 is already in use, trying 8080.");
    
    WLog::error("Failed to connect to the database.");
    
    std::cout << std::endl;
    WLog::info("Application finished.");

    return 0;
}
