// src/http/Request.h
#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include <sstream>
#include <algorithm>

enum class HttpMethod {
    GET,
    POST,
    HEAD,
    PUT,
    DELETE,
    UNKNOWN
};

class HttpRequest {
private:
    HttpMethod method;
    std::string path;
    std::string version;
    std::unordered_map<std::string, std::string> headers;
    std::string body;
    std::unordered_map<std::string, std::string> queryParams;
    
    void parseQueryString(const std::string& query);
    
public:
    HttpRequest() : method(HttpMethod::UNKNOWN) {}
    
    bool parse(const std::string& rawRequest);
    
    // Getters
    HttpMethod getMethod() const { return method; }
    std::string getPath() const { return path; }
    std::string getVersion() const { return version; }
    std::string getHeader(const std::string& key) const;
    std::string getBody() const { return body; }
    
    // Utility
    std::string getContentType() const;
    size_t getContentLength() const;
    std::string getQueryParam(const std::string& key) const;
    
    static HttpMethod stringToMethod(const std::string& str);
    static std::string urlDecode(const std::string& str);
};