// src/config/Config.cpp
#include "Config.h"
#include <fstream>
#include <iostream>
#include <algorithm>
#include <cctype>

bool Config::loadFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        return false;
    }
    
    std::string line;
    std::string currentSection;
    
    while (std::getline(file, line)) {
        line = trim(line);
        
        // Skip empty lines and comments
        if (line.empty() || line[0] == '#' || line[0] == ';') {
            continue;
        }
        
        // Check for section header
        if (line[0] == '[' && line.back() == ']') {
            currentSection = line.substr(1, line.length() - 2);
            continue;
        }
        
        // Parse key-value pair
        size_t equalPos = line.find('=');
        if (equalPos != std::string::npos) {
            std::string key = trim(line.substr(0, equalPos));
            std::string value = trim(line.substr(equalPos + 1));
            
            // Add section prefix if present
            if (!currentSection.empty()) {
                key = currentSection + "." + key;
            }
            
            settings[key] = value;
        }
    }
    
    file.close();
    return true;
}

bool Config::loadFromArgs(int argc, char* argv[]) {
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg.substr(0, 2) == "--") {
            size_t equalPos = arg.find('=');
            if (equalPos != std::string::npos) {
                std::string key = arg.substr(2, equalPos - 2);
                std::string value = arg.substr(equalPos + 1);
                settings[key] = value;
            } else if (i + 1 < argc && argv[i + 1][0] != '-') {
                std::string key = arg.substr(2);
                std::string value = argv[++i];
                settings[key] = value;
            }
        }
    }
    return true;
}

int Config::getInt(const std::string& key, int defaultValue) {
    auto it = settings.find(key);
    if (it != settings.end()) {
        try {
            return std::stoi(it->second);
        } catch (...) {
            return defaultValue;
        }
    }
    return defaultValue;
}

std::string Config::getString(const std::string& key, const std::string& defaultValue) {
    auto it = settings.find(key);
    if (it != settings.end()) {
        return it->second;
    }
    return defaultValue;
}

bool Config::getBool(const std::string& key, bool defaultValue) {
    auto it = settings.find(key);
    if (it != settings.end()) {
        std::string value = it->second;
        std::transform(value.begin(), value.end(), value.begin(), ::tolower);
        return (value == "true" || value == "yes" || value == "1" || value == "on");
    }
    return defaultValue;
}

void Config::set(const std::string& key, const std::string& value) {
    settings[key] = value;
}

Config Config::getDefault() {
    Config config;
    
    // Server settings
    config.set("server.port", "8080");
    config.set("server.max_threads", "4");
    config.set("server.max_connections", "100");
    config.set("server.timeout", "30");
    config.set("server.web_root", "./www");
    
    // Security settings
    config.set("security.enable_directory_listing", "false");
    config.set("security.default_index", "index.html");
    config.set("security.max_file_size", "10485760"); // 10MB
    
    // Logging settings
    config.set("logging.level", "INFO");
    config.set("logging.file", "server.log");
    config.set("logging.console", "true");
    
    return config;
}

void Config::printAll() const {
    for (const auto& pair : settings) {
        std::cout << pair.first << " = " << pair.second << std::endl;
    }
}

std::string Config::trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return "";
    
    size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, last - first + 1);
}