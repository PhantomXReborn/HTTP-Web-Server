// src/socket/Socket.cpp
#include "Socket.h"
#include <iostream>

#ifdef _WIN32
    static bool wsaInitialized = false;
#endif

Socket::Socket() : sockfd(INVALID_SOCKET_VALUE) {
    initializeNetwork();
}

Socket::~Socket() {
    close();
}

void Socket::initializeNetwork() {
    #ifdef _WIN32
        if (!wsaInitialized) {
            WSADATA wsaData;
            if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
                throw std::runtime_error("WSAStartup failed");
            }
            wsaInitialized = true;
        }
    #endif
}

void Socket::cleanupNetwork() {
    #ifdef _WIN32
        if (wsaInitialized) {
            WSACleanup();
            wsaInitialized = false;
        }
    #endif
}

bool Socket::create() {
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == INVALID_SOCKET_VALUE) {
        return false;
    }
    
    // Allow socket reuse
    int opt = 1;
    #ifdef _WIN32
        setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));
    #else
        setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    #endif
    
    return true;
}

bool Socket::bind(int port) {
    if (sockfd == INVALID_SOCKET_VALUE) return false;
    
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);
    
    #ifdef _WIN32
        return (::bind(sockfd, (struct sockaddr*)&address, sizeof(address)) != SOCKET_ERROR);
    #else
        return (::bind(sockfd, (struct sockaddr*)&address, sizeof(address)) >= 0);
    #endif
}

bool Socket::listen(int backlog) {
    if (sockfd == INVALID_SOCKET_VALUE) return false;
    
    #ifdef _WIN32
        return (::listen(sockfd, backlog) != SOCKET_ERROR);
    #else
        return (::listen(sockfd, backlog) >= 0);
    #endif
}

int Socket::accept(std::string& clientIP) {
    if (sockfd == INVALID_SOCKET_VALUE) return -1;
    
    struct sockaddr_in clientAddr;
    #ifdef _WIN32
        int clientLen = sizeof(clientAddr);
    #else
        socklen_t clientLen = sizeof(clientAddr);
    #endif
    
    SocketHandle clientSocket = ::accept(sockfd, (struct sockaddr*)&clientAddr, &clientLen);
    
    if (clientSocket == INVALID_SOCKET_VALUE) {
        return -1;
    }
    
    char ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &clientAddr.sin_addr, ip, INET_ADDRSTRLEN);
    clientIP = std::string(ip);
    
    return clientSocket;
}

ssize_t Socket::send(const std::string& data) {
    if (sockfd == INVALID_SOCKET_VALUE) return -1;
    
    #ifdef _WIN32
        return ::send(sockfd, data.c_str(), (int)data.length(), 0);
    #else
        return ::send(sockfd, data.c_str(), data.length(), 0);
    #endif
}

ssize_t Socket::receive(std::string& data, size_t size) {
    if (sockfd == INVALID_SOCKET_VALUE) return -1;
    
    char* buffer = new char[size];
    memset(buffer, 0, size);
    
    #ifdef _WIN32
        ssize_t bytesReceived = ::recv(sockfd, buffer, (int)size, 0);
    #else
        ssize_t bytesReceived = ::recv(sockfd, buffer, size, 0);
    #endif
    
    if (bytesReceived > 0) {
        data.assign(buffer, bytesReceived);
    }
    
    delete[] buffer;
    return bytesReceived;
}

void Socket::close() {
    if (sockfd != INVALID_SOCKET_VALUE) {
        #ifdef _WIN32
            closesocket(sockfd);
        #else
            ::close(sockfd);
        #endif
        sockfd = INVALID_SOCKET_VALUE;
    }
}