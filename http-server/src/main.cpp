// src/main.cpp
#include "server/Server.h"
#include "config/Config.h"
#include "utils/Logger.h"
#include <iostream>
#include <csignal>
#include <cstdlib>

HttpServer* serverPtr = nullptr;

void signalHandler(int signum) {
    std::cout << "\nReceived signal " << signum << ". Shutting down server...\n";
    if (serverPtr) {
        serverPtr->stop();
    }
    Logger::close();
    exit(signum);
}

void printHelp() {
    std::cout << "Usage: httpserver [options]\n";
    std::cout << "Options:\n";
    std::cout << "  --port=<port>          Port to listen on (default: 8080)\n";
    std::cout << "  --web_root=<path>      Web root directory (default: ./www)\n";
    std::cout << "  --config=<file>        Configuration file\n";
    std::cout << "  --max_threads=<num>    Maximum worker threads (default: 4)\n";
    std::cout << "  --help                 Show this help message\n";
}

int main(int argc, char* argv[]) {
    // Parse command line arguments for help
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "--help" || arg == "-h") {
            printHelp();
            return 0;
        }
    }
    
    // Setup signal handling
    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);
    
    try {
        // Initialize logger
        Logger::init("server.log", LogLevel::INFO);
        
        // Create and initialize server
        HttpServer server;
        serverPtr = &server;
        
        // Load configuration
        Config config;
        bool configLoaded = false;
        
        // Check for config file in arguments
        for (int i = 1; i < argc; i++) {
            std::string arg = argv[i];
            if (arg.find("--config=") == 0) {
                std::string configFile = arg.substr(9);
                if (config.loadFromFile(configFile)) {
                    configLoaded = true;
                    Logger::info("Loaded configuration from: " + configFile);
                } else {
                    Logger::warning("Cannot load config file: " + configFile);
                }
                break;
            }
        }
        
        // If no config file, use defaults and parse command line
        if (!configLoaded) {
            config = Config::getDefault();
            config.loadFromArgs(argc, argv);
        }
        
        // Update logger settings from config
        std::string logLevel = config.getString("logging.level", "INFO");
        if (logLevel == "DEBUG") Logger::setLogLevel(LogLevel::DEBUG);
        else if (logLevel == "WARNING") Logger::setLogLevel(LogLevel::WARNING);
        else if (logLevel == "ERROR") Logger::setLogLevel(LogLevel::ERROR);
        
        // Initialize server with configuration
        if (!server.initialize()) {
            Logger::error("Failed to initialize server");
            return 1;
        }
        
        Logger::info("HTTP Server starting...");
        Logger::info("Web root: " + config.getString("server.web_root", "./www"));
        Logger::info("Port: " + std::to_string(config.getInt("server.port", 8080)));
        Logger::info("Threads: " + std::to_string(config.getInt("server.max_threads", 4)));
        Logger::info("Press Ctrl+C to stop the server");
        
        // Start server (this will block until stopped)
        server.start();
        
        Logger::info("Server shutdown complete");
        
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}