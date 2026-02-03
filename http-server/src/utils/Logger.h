// src/utils/Logger.h
#pragma once
#include <string>
#include <fstream>
#include <iostream>
#include <mutex>
#include <chrono>
#include <iomanip>

enum class LogLevel {
    DEBUG,
    INFO,
    WARNING,
    ERROR
};

class Logger {
private:
    static std::ofstream logFile;
    static LogLevel currentLevel;
    static std::mutex logMutex;
    
    static std::string levelToString(LogLevel level);
    static std::string getCurrentTime();
    
public:
    static void init(const std::string& filename = "", LogLevel level = LogLevel::INFO);
    static void close();
    
    static void debug(const std::string& message);
    static void info(const std::string& message);
    static void warning(const std::string& message);
    static void error(const std::string& message);
    
    static void setLogLevel(LogLevel level) { currentLevel = level; }
};