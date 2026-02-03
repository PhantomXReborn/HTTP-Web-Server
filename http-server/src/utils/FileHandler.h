// src/utils/FileHandler.h
#pragma once
#include <string>
#include <vector>
#include <fstream>
#include <filesystem>

class FileHandler {
public:
    static bool fileExists(const std::string& path);
    static std::string readFile(const std::string& path);
    static bool writeFile(const std::string& path, const std::string& content);
    static std::string getMimeType(const std::string& filename);
    static size_t getFileSize(const std::string& path);
    
    // Directory operations
    static bool isDirectory(const std::string& path);
    static std::vector<std::string> listDirectory(const std::string& path);
    
    // Security
    static bool isPathSafe(const std::string& webRoot, const std::string& requestedPath);
    
    // File utilities
    static std::string getFileExtension(const std::string& filename);
    static std::string normalizePath(const std::string& path);
};