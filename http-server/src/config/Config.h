// src/config/Config.h
#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include <sstream>

class Config {
private:
    std::unordered_map<std::string, std::string> settings;
    
    void parseLine(const std::string& line);
    std::string trim(const std::string& str);
    
public:
    Config() = default;
    
    bool loadFromFile(const std::string& filename);
    bool loadFromArgs(int argc, char* argv[]);
    
    // Getters with defaults
    int getInt(const std::string& key, int defaultValue = 0);
    std::string getString(const std::string& key, const std::string& defaultValue = "");
    bool getBool(const std::string& key, bool defaultValue = false);
    
    // Setter
    void set(const std::string& key, const std::string& value);
    
    // Default configuration
    static Config getDefault();
    
    // Debug
    void printAll() const;
};