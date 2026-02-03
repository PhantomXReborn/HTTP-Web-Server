// src/http/Response.h
#pragma once
#include <string>
#include <unordered_map>
#include <ctime>

class HttpResponse {
private:
    int statusCode;
    std::string statusMessage;
    std::unordered_map<std::string, std::string> headers;
    std::string body;
    
    static std::string getStatusMessage(int code);
    
public:
    HttpResponse() : statusCode(200) {
        setDefaultHeaders();
    }
    
    // Builder pattern methods
    HttpResponse& setStatusCode(int code);
    HttpResponse& setStatusMessage(const std::string& message);
    HttpResponse& setHeader(const std::string& key, const std::string& value);
    HttpResponse& setBody(const std::string& bodyContent);
    HttpResponse& setContentType(const std::string& type);
    
    // Generate response string
    std::string toString() const;
    
    // Common responses
    static HttpResponse makeErrorResponse(int code, const std::string& message);
    static HttpResponse makeFileResponse(const std::string& fileContent, const std::string& contentType);
    static HttpResponse makeTextResponse(const std::string& text);
    static HttpResponse makeRedirectResponse(const std::string& location);
    
private:
    void setDefaultHeaders();
    static std::string getMimeType(const std::string& extension);
    static std::string getCurrentTime();
};