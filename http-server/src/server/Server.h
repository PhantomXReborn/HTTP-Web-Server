// src/server/Server.h
#pragma once
#include "Socket.h"
#include "../http/Request.h"
#include "../http/Response.h"
#include "../config/Config.h"
#include "../utils/FileHandler.h"
#include "../utils/Logger.h"
#include <atomic>
#include <memory>
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>

class HttpServer {
private:
    // Thread Pool Implementation (now internal to Server class)
    class ThreadPool {
    private:
        std::vector<std::thread> workers;
        std::queue<std::function<void()>> tasks;
        std::mutex queueMutex;
        std::condition_variable condition;
        std::atomic<bool> stop;
        
    public:
        ThreadPool(size_t threads) : stop(false) {
            for(size_t i = 0; i < threads; ++i) {
                workers.emplace_back([this] {
                    while(true) {
                        std::function<void()> task;
                        {
                            std::unique_lock<std::mutex> lock(this->queueMutex);
                            this->condition.wait(lock, [this] {
                                return this->stop || !this->tasks.empty();
                            });
                            if(this->stop && this->tasks.empty())
                                return;
                            task = std::move(this->tasks.front());
                            this->tasks.pop();
                        }
                        task();
                    }
                });
            }
        }
        
        ~ThreadPool() {
            {
                std::unique_lock<std::mutex> lock(queueMutex);
                stop = true;
            }
            condition.notify_all();
            for(std::thread &worker: workers)
                worker.join();
        }
        
        template<class F>
        void enqueue(F&& task) {
            {
                std::unique_lock<std::mutex> lock(queueMutex);
                if(stop)
                    throw std::runtime_error("enqueue on stopped ThreadPool");
                tasks.emplace(std::forward<F>(task));
            }
            condition.notify_one();
        }
    };
    
    // Server members
    std::unique_ptr<Socket> serverSocket;
    std::unique_ptr<ThreadPool> threadPool;
    Config config;
    std::atomic<bool> running;
    std::string webRoot;
    
public:
    HttpServer() : running(false) {}
    ~HttpServer() { stop(); }
    
    bool initialize(const std::string& configPath = "") {
        try {
            // Load configuration
            if (!configPath.empty()) {
                if (!config.loadFromFile(configPath)) {
                    Logger::error("Failed to load config file: " + configPath);
                    return false;
                }
            } else {
                config = Config::getDefault();
            }
            
            // Get configuration values
            int port = config.getInt("server.port", 8080);
            int maxThreads = config.getInt("server.max_threads", 4);
            webRoot = config.getString("server.web_root", "./www");
            
            // Initialize socket
            serverSocket = std::make_unique<Socket>();
            if (!serverSocket->create()) {
                Logger::error("Failed to create socket");
                return false;
            }
            
            if (!serverSocket->bind(port)) {
                Logger::error("Failed to bind to port " + std::to_string(port));
                return false;
            }
            
            if (!serverSocket->listen()) {
                Logger::error("Failed to listen on socket");
                return false;
            }
            
            // Initialize thread pool
            threadPool = std::make_unique<ThreadPool>(maxThreads);
            
            // Create web root directory if it doesn't exist
            if (!FileHandler::isDirectory(webRoot)) {
                // Create directory (you'll need to implement this)
                Logger::warning("Web root directory doesn't exist: " + webRoot);
            }
            
            Logger::info("Server initialized successfully");
            Logger::info("Port: " + std::to_string(port));
            Logger::info("Web root: " + webRoot);
            Logger::info("Threads: " + std::to_string(maxThreads));
            
            return true;
            
        } catch (const std::exception& e) {
            Logger::error("Initialization error: " + std::string(e.what()));
            return false;
        }
    }
    
    void start() {
        if (!serverSocket) {
            Logger::error("Server not initialized");
            return;
        }
        
        running = true;
        Logger::info("Server started. Listening for connections...");
        
        while (running) {
            std::string clientIP;
            int clientSocket = serverSocket->accept(clientIP);
            
            if (clientSocket < 0) {
                if (running) {
                    Logger::error("Failed to accept connection");
                }
                continue;
            }
            
            Logger::debug("New connection from: " + clientIP);
            
            // Handle client in thread pool
            threadPool->enqueue([this, clientSocket, clientIP]() {
                handleClient(clientSocket, clientIP);
            });
        }
    }
    
    void stop() {
        running = false;
        if (serverSocket) {
            serverSocket->close();
        }
        Logger::info("Server stopped");
    }
    
private:
    void handleClient(int clientSocket, const std::string& clientIP) {
        try {
            // Receive request
            std::string requestData;
            ssize_t bytesReceived = 0;
            char buffer[4096];
            
            do {
                memset(buffer, 0, sizeof(buffer));
                bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
                
                if (bytesReceived > 0) {
                    requestData.append(buffer, bytesReceived);
                    
                    // Check if we have received the complete request
                    if (requestData.find("\r\n\r\n") != std::string::npos) {
                        // Check for Content-Length if it's a POST request
                        size_t contentLength = 0;
                        size_t headerEnd = requestData.find("\r\n\r\n");
                        std::string headers = requestData.substr(0, headerEnd);
                        
                        size_t clPos = headers.find("Content-Length:");
                        if (clPos != std::string::npos) {
                            size_t clStart = clPos + 15;
                            size_t clEnd = headers.find("\r\n", clStart);
                            std::string clStr = headers.substr(clStart, clEnd - clStart);
                            contentLength = std::stoul(clStr);
                            
                            // Wait for complete body
                            size_t bodyStart = headerEnd + 4;
                            if (requestData.length() - bodyStart < contentLength) {
                                continue; // Need more data
                            }
                        }
                        break; // We have complete request
                    }
                } else if (bytesReceived == 0) {
                    Logger::debug("Client disconnected: " + clientIP);
                    break;
                } else {
                    Logger::error("Error receiving data from: " + clientIP);
                    break;
                }
            } while (true);
            
            if (!requestData.empty()) {
                processRequest(clientSocket, requestData);
            }
            
        } catch (const std::exception& e) {
            Logger::error("Error handling client " + clientIP + ": " + e.what());
        }
        
        // Close client socket
        #ifdef _WIN32
            closesocket(clientSocket);
        #else
            close(clientSocket);
        #endif
    }
    
    void processRequest(int clientSocket, const std::string& rawRequest) {
        try {
            HttpRequest request;
            if (!request.parse(rawRequest)) {
                // Send 400 Bad Request
                HttpResponse badRequest = HttpResponse::makeErrorResponse(400, "Bad Request");
                sendResponse(clientSocket, badRequest.toString());
                return;
            }
            
            // Route request based on method
            switch (request.getMethod()) {
                case HttpMethod::GET:
                    handleGet(clientSocket, request);
                    break;
                case HttpMethod::POST:
                    handlePost(clientSocket, request);
                    break;
                case HttpMethod::HEAD:
                    handleHead(clientSocket, request);
                    break;
                default:
                    // Send 501 Not Implemented
                    HttpResponse notImplemented = HttpResponse::makeErrorResponse(501, "Not Implemented");
                    sendResponse(clientSocket, notImplemented.toString());
            }
            
        } catch (const std::exception& e) {
            Logger::error("Error processing request: " + std::string(e.what()));
            HttpResponse error = HttpResponse::makeErrorResponse(500, "Internal Server Error");
            sendResponse(clientSocket, error.toString());
        }
    }
    
    void handleGet(int clientSocket, const HttpRequest& request) {
        std::string path = request.getPath();
        
        // Default to index.html if root path
        if (path == "/") {
            path = "/index.html";
        }
        
        // Get safe file path
        std::string filePath = webRoot + path;
        
        if (!FileHandler::isPathSafe(webRoot, filePath)) {
            HttpResponse response = HttpResponse::makeErrorResponse(403, "Forbidden");
            sendResponse(clientSocket, response.toString());
            return;
        }
        
        if (!FileHandler::fileExists(filePath)) {
            HttpResponse response = HttpResponse::makeErrorResponse(404, "Not Found");
            sendResponse(clientSocket, response.toString());
            return;
        }
        
        // Check if it's a directory
        if (FileHandler::isDirectory(filePath)) {
            bool enableListing = config.getBool("security.enable_directory_listing", false);
            std::string defaultIndex = config.getString("security.default_index", "index.html");
            
            // Try default index file
            std::string indexFile = filePath + "/" + defaultIndex;
            if (FileHandler::fileExists(indexFile)) {
                filePath = indexFile;
            } else if (enableListing) {
                // Generate directory listing
                std::string listing = generateDirectoryListing(filePath, path);
                HttpResponse response;
                response.setStatusCode(200);
                response.setStatusMessage("OK");
                response.setContentType("text/html");
                response.setBody(listing);
                sendResponse(clientSocket, response.toString());
                return;
            } else {
                HttpResponse response = HttpResponse::makeErrorResponse(403, "Forbidden");
                sendResponse(clientSocket, response.toString());
                return;
            }
        }
        
        // Serve the file
        std::string fileContent = FileHandler::readFile(filePath);
        HttpResponse response;
        response.setStatusCode(200);
        response.setStatusMessage("OK");
        response.setContentType(FileHandler::getMimeType(filePath));
        response.setHeader("Content-Length", std::to_string(fileContent.size()));
        response.setBody(fileContent);
        
        sendResponse(clientSocket, response.toString());
    }
    
    void handlePost(int clientSocket, const HttpRequest& request) {
        // Simple echo server for now
        HttpResponse response;
        response.setStatusCode(200);
        response.setStatusMessage("OK");
        response.setContentType("text/plain");
        response.setBody("Received POST request with body: " + request.getBody());
        
        sendResponse(clientSocket, response.toString());
    }
    
    void handleHead(int clientSocket, const HttpRequest& request) {
        // Similar to GET but without body
        std::string path = request.getPath();
        if (path == "/") {
            path = "/index.html";
        }
        
        std::string filePath = webRoot + path;
        
        if (!FileHandler::isPathSafe(webRoot, filePath) || !FileHandler::fileExists(filePath)) {
            HttpResponse response = HttpResponse::makeErrorResponse(404, "Not Found");
            sendResponse(clientSocket, response.toString());
            return;
        }
        
        HttpResponse response;
        response.setStatusCode(200);
        response.setStatusMessage("OK");
        response.setContentType(FileHandler::getMimeType(filePath));
        response.setHeader("Content-Length", std::to_string(FileHandler::getFileSize(filePath)));
        
        sendResponse(clientSocket, response.toString());
    }
    
    void sendResponse(int clientSocket, const std::string& response) {
        ssize_t bytesSent = send(clientSocket, response.c_str(), response.length(), 0);
        if (bytesSent < 0) {
            Logger::error("Failed to send response");
        }
    }
    
    std::string generateDirectoryListing(const std::string& dirPath, const std::string& urlPath) {
        std::vector<std::string> files = FileHandler::listDirectory(dirPath);
        
        std::string html = "<!DOCTYPE html>\n";
        html += "<html><head><title>Directory Listing</title></head>\n";
        html += "<body>\n";
        html += "<h1>Directory Listing: " + urlPath + "</h1>\n";
        html += "<ul>\n";
        
        // Parent directory link
        if (urlPath != "/") {
            size_t lastSlash = urlPath.find_last_of('/');
            std::string parentPath = urlPath.substr(0, lastSlash);
            if (parentPath.empty()) parentPath = "/";
            html += "<li><a href=\"" + parentPath + "\">../</a></li>\n";
        }
        
        // List files
        for (const auto& file : files) {
            html += "<li><a href=\"" + urlPath + (urlPath == "/" ? "" : "/") + file + "\">" + file + "</a></li>\n";
        }
        
        html += "</ul>\n";
        html += "</body></html>";
        
        return html;
    }
};