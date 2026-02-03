// src/http/Request.cpp
#include "Request.h"
#include <iostream>

bool HttpRequest::parse(const std::string& rawRequest) {
    std::istringstream requestStream(rawRequest);
    std::string line;
    
    // Parse request line
    if (!std::getline(requestStream, line)) {
        return false;
    }
    
    std::istringstream lineStream(line);
    std::string methodStr, fullPath;
    lineStream >> methodStr >> fullPath >> version;
    
    method = stringToMethod(methodStr);
    
    // Separate path and query string
    size_t queryPos = fullPath.find('?');
    if (queryPos != std::string::npos) {
        path = fullPath.substr(0, queryPos);
        std::string query = fullPath.substr(queryPos + 1);
        parseQueryString(query);
    } else {
        path = fullPath;
    }
    
    // Parse headers
    while (std::getline(requestStream, line) && line != "\r" && line != "") {
        size_t colonPos = line.find(':');
        if (colonPos != std::string::npos) {
            std::string key = line.substr(0, colonPos);
            std::string value = line.substr(colonPos + 2); // Skip ": "
            
            // Remove trailing \r
            if (!value.empty() && value.back() == '\r') {
                value.pop_back();
            }
            
            headers[key] = value;
        }
    }
    
    // Parse body
    if (getContentLength() > 0) {
        size_t contentLen = getContentLength();
        char* bodyBuffer = new char[contentLen + 1];
        requestStream.read(bodyBuffer, contentLen);
        bodyBuffer[contentLen] = '\0';
        body = std::string(bodyBuffer);
        delete[] bodyBuffer;
    }
    
    return true;
}

void HttpRequest::parseQueryString(const std::string& query) {
    std::istringstream queryStream(query);
    std::string pair;
    
    while (std::getline(queryStream, pair, '&')) {
        size_t equalPos = pair.find('=');
        if (equalPos != std::string::npos) {
            std::string key = pair.substr(0, equalPos);
            std::string value = pair.substr(equalPos + 1);
            queryParams[urlDecode(key)] = urlDecode(value);
        }
    }
}

std::string HttpRequest::getHeader(const std::string& key) const {
    auto it = headers.find(key);
    if (it != headers.end()) {
        return it->second;
    }
    return "";
}

std::string HttpRequest::getContentType() const {
    return getHeader("Content-Type");
}

size_t HttpRequest::getContentLength() const {
    std::string cl = getHeader("Content-Length");
    if (!cl.empty()) {
        try {
            return std::stoul(cl);
        } catch (...) {
            return 0;
        }
    }
    return 0;
}

std::string HttpRequest::getQueryParam(const std::string& key) const {
    auto it = queryParams.find(key);
    if (it != queryParams.end()) {
        return it->second;
    }
    return "";
}

HttpMethod HttpRequest::stringToMethod(const std::string& str) {
    if (str == "GET") return HttpMethod::GET;
    if (str == "POST") return HttpMethod::POST;
    if (str == "HEAD") return HttpMethod::HEAD;
    if (str == "PUT") return HttpMethod::PUT;
    if (str == "DELETE") return HttpMethod::DELETE;
    return HttpMethod::UNKNOWN;
}

std::string HttpRequest::urlDecode(const std::string& str) {
    std::string result;
    for (size_t i = 0; i < str.length(); ++i) {
        if (str[i] == '%' && i + 2 < str.length()) {
            std::string hex = str.substr(i + 1, 2);
            char ch = static_cast<char>(std::stoi(hex, nullptr, 16));
            result += ch;
            i += 2;
        } else if (str[i] == '+') {
            result += ' ';
        } else {
            result += str[i];
        }
    }
    return result;
}