// src/utils/FileHandler.cpp
#include "FileHandler.h"
#include <iostream>
#include <stdexcept>

namespace fs = std::filesystem;

bool FileHandler::fileExists(const std::string& path) {
    try {
        return fs::exists(path) && fs::is_regular_file(path);
    } catch (...) {
        return false;
    }
}

std::string FileHandler::readFile(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open file: " + path);
    }
    
    std::string content;
    file.seekg(0, std::ios::end);
    content.resize(file.tellg());
    file.seekg(0, std::ios::beg);
    file.read(&content[0], content.size());
    file.close();
    
    return content;
}

bool FileHandler::writeFile(const std::string& path, const std::string& content) {
    std::ofstream file(path, std::ios::binary);
    if (!file.is_open()) {
        return false;
    }
    
    file.write(content.c_str(), content.size());
    file.close();
    return true;
}

std::string FileHandler::getMimeType(const std::string& filename) {
    std::string ext = getFileExtension(filename);
    
    static const std::unordered_map<std::string, std::string> mimeTypes = {
        {".html", "text/html"},
        {".htm", "text/html"},
        {".css", "text/css"},
        {".js", "application/javascript"},
        {".json", "application/json"},
        {".png", "image/png"},
        {".jpg", "image/jpeg"},
        {".jpeg", "image/jpeg"},
        {".gif", "image/gif"},
        {".svg", "image/svg+xml"},
        {".txt", "text/plain"},
        {".pdf", "application/pdf"},
        {".zip", "application/zip"},
        {".xml", "application/xml"},
        {".ico", "image/x-icon"}
    };
    
    auto it = mimeTypes.find(ext);
    if (it != mimeTypes.end()) {
        return it->second;
    }
    return "application/octet-stream";
}

size_t FileHandler::getFileSize(const std::string& path) {
    try {
        return fs::file_size(path);
    } catch (...) {
        return 0;
    }
}

bool FileHandler::isDirectory(const std::string& path) {
    try {
        return fs::is_directory(path);
    } catch (...) {
        return false;
    }
}

std::vector<std::string> FileHandler::listDirectory(const std::string& path) {
    std::vector<std::string> files;
    try {
        for (const auto& entry : fs::directory_iterator(path)) {
            std::string filename = entry.path().filename().string();
            if (entry.is_directory()) {
                filename += "/";
            }
            files.push_back(filename);
        }
    } catch (...) {
        // Return empty vector on error
    }
    return files;
}

bool FileHandler::isPathSafe(const std::string& webRoot, const std::string& requestedPath) {
    try {
        fs::path root = fs::absolute(webRoot);
        fs::path request = fs::absolute(requestedPath);
        
        // Check if requested path is within web root
        auto rootIt = root.begin();
        auto reqIt = request.begin();
        
        while (rootIt != root.end() && reqIt != request.end()) {
            if (*rootIt != *reqIt) {
                return false;
            }
            ++rootIt;
            ++reqIt;
        }
        
        // If we've gone through all root components, path is safe
        return rootIt == root.end();
    } catch (...) {
        return false;
    }
}

std::string FileHandler::getFileExtension(const std::string& filename) {
    size_t dotPos = filename.find_last_of('.');
    if (dotPos != std::string::npos) {
        return filename.substr(dotPos);
    }
    return "";
}

std::string FileHandler::normalizePath(const std::string& path) {
    try {
        return fs::absolute(fs::path(path)).string();
    } catch (...) {
        return path;
    }
}