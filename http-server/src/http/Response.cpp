// src/http/Response.cpp
#include "Response.h"
#include <sstream>
#include <map>

HttpResponse& HttpResponse::setStatusCode(int code) {
    statusCode = code;
    statusMessage = getStatusMessage(code);
    return *this;
}

HttpResponse& HttpResponse::setStatusMessage(const std::string& message) {
    statusMessage = message;
    return *this;
}

HttpResponse& HttpResponse::setHeader(const std::string& key, const std::string& value) {
    headers[key] = value;
    return *this;
}

HttpResponse& HttpResponse::setBody(const std::string& bodyContent) {
    body = bodyContent;
    setHeader("Content-Length", std::to_string(body.length()));
    return *this;
}

HttpResponse& HttpResponse::setContentType(const std::string& type) {
    setHeader("Content-Type", type);
    return *this;
}

void HttpResponse::setDefaultHeaders() {
    headers["Server"] = "C++ HTTP Server";
    headers["Date"] = getCurrentTime();
    headers["Connection"] = "close";
}

std::string HttpResponse::toString() const {
    std::ostringstream response;
    
    // Status line
    response << "HTTP/1.1 " << statusCode << " " << statusMessage << "\r\n";
    
    // Headers
    for (const auto& header : headers) {
        response << header.first << ": " << header.second << "\r\n";
    }
    
    // Empty line separating headers and body
    response << "\r\n";
    
    // Body
    if (!body.empty()) {
        response << body;
    }
    
    return response.str();
}

HttpResponse HttpResponse::makeErrorResponse(int code, const std::string& message) {
    HttpResponse response;
    response.setStatusCode(code);
    
    std::string html = "<!DOCTYPE html>\n";
    html += "<html><head><title>" + std::to_string(code) + " " + message + "</title></head>\n";
    html += "<body>\n";
    html += "<h1>" + std::to_string(code) + " " + message + "</h1>\n";
    html += "<hr>\n";
    html += "<p>C++ HTTP Server</p>\n";
    html += "</body></html>";
    
    response.setContentType("text/html");
    response.setBody(html);
    
    return response;
}

HttpResponse HttpResponse::makeFileResponse(const std::string& fileContent, const std::string& contentType) {
    HttpResponse response;
    response.setStatusCode(200);
    response.setContentType(contentType);
    response.setBody(fileContent);
    return response;
}

HttpResponse HttpResponse::makeTextResponse(const std::string& text) {
    HttpResponse response;
    response.setStatusCode(200);
    response.setContentType("text/plain");
    response.setBody(text);
    return response;
}

HttpResponse HttpResponse::makeRedirectResponse(const std::string& location) {
    HttpResponse response;
    response.setStatusCode(302);
    response.setHeader("Location", location);
    response.setContentType("text/html");
    response.setBody("<html><body>Redirecting to <a href=\"" + location + "\">" + location + "</a></body></html>");
    return response;
}

std::string HttpResponse::getStatusMessage(int code) {
    static const std::map<int, std::string> statusMessages = {
        {200, "OK"},
        {201, "Created"},
        {204, "No Content"},
        {301, "Moved Permanently"},
        {302, "Found"},
        {304, "Not Modified"},
        {400, "Bad Request"},
        {401, "Unauthorized"},
        {403, "Forbidden"},
        {404, "Not Found"},
        {405, "Method Not Allowed"},
        {500, "Internal Server Error"},
        {501, "Not Implemented"},
        {503, "Service Unavailable"}
    };
    
    auto it = statusMessages.find(code);
    if (it != statusMessages.end()) {
        return it->second;
    }
    return "Unknown Status";
}

std::string HttpResponse::getMimeType(const std::string& extension) {
    static const std::map<std::string, std::string> mimeTypes = {
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
        {".xml", "application/xml"}
    };
    
    auto it = mimeTypes.find(extension);
    if (it != mimeTypes.end()) {
        return it->second;
    }
    return "application/octet-stream";
}

std::string HttpResponse::getCurrentTime() {
    time_t now = time(nullptr);
    struct tm* timeinfo = gmtime(&now);
    
    char buffer[80];
    strftime(buffer, sizeof(buffer), "%a, %d %b %Y %H:%M:%S GMT", timeinfo);
    
    return std::string(buffer);
}