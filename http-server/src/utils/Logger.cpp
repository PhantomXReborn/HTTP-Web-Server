// src/utils/Logger.cpp
#include "Logger.h"

std::ofstream Logger::logFile;
LogLevel Logger::currentLevel = LogLevel::INFO;
std::mutex Logger::logMutex;

void Logger::init(const std::string& filename, LogLevel level) {
    std::lock_guard<std::mutex> lock(logMutex);
    
    currentLevel = level;
    
    if (!filename.empty()) {
        logFile.open(filename, std::ios::app);
        if (!logFile.is_open()) {
            std::cerr << "Warning: Cannot open log file: " << filename << std::endl;
        }
    }
}

void Logger::close() {
    std::lock_guard<std::mutex> lock(logMutex);
    if (logFile.is_open()) {
        logFile.close();
    }
}

std::string Logger::levelToString(LogLevel level) {
    switch (level) {
        case LogLevel::DEBUG: return "DEBUG";
        case LogLevel::INFO: return "INFO";
        case LogLevel::WARNING: return "WARNING";
        case LogLevel::ERROR: return "ERROR";
        default: return "UNKNOWN";
    }
}

std::string Logger::getCurrentTime() {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

void Logger::debug(const std::string& message) {
    if (currentLevel <= LogLevel::DEBUG) {
        log(LogLevel::DEBUG, message);
    }
}

void Logger::info(const std::string& message) {
    if (currentLevel <= LogLevel::INFO) {
        log(LogLevel::INFO, message);
    }
}

void Logger::warning(const std::string& message) {
    if (currentLevel <= LogLevel::WARNING) {
        log(LogLevel::WARNING, message);
    }
}

void Logger::error(const std::string& message) {
    if (currentLevel <= LogLevel::ERROR) {
        log(LogLevel::ERROR, message);
    }
}

void Logger::log(LogLevel level, const std::string& message) {
    std::lock_guard<std::mutex> lock(logMutex);
    
    std::string logEntry = "[" + getCurrentTime() + "] [" + levelToString(level) + "] " + message;
    
    // Always print to console
    if (level >= LogLevel::WARNING) {
        std::cerr << logEntry << std::endl;
    } else {
        std::cout << logEntry << std::endl;
    }
    
    // Write to file if open
    if (logFile.is_open()) {
        logFile << logEntry << std::endl;
        logFile.flush();
    }
}