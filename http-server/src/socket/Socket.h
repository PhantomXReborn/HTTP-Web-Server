// src/socket/Socket.h
#pragma once
#include <string>
#include <stdexcept>

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
    typedef SOCKET SocketHandle;
    #define SOCKET_ERROR_VALUE SOCKET_ERROR
    #define INVALID_SOCKET_VALUE INVALID_SOCKET
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <unistd.h>
    #include <arpa/inet.h>
    #include <cstring>
    typedef int SocketHandle;
    #define SOCKET_ERROR_VALUE -1
    #define INVALID_SOCKET_VALUE -1
    #define closesocket close
#endif

class Socket {
private:
    SocketHandle sockfd;
    struct sockaddr_in address;
    
public:
    Socket();
    ~Socket();
    
    bool create();
    bool bind(int port);
    bool listen(int backlog = 5);
    int accept(std::string& clientIP);
    bool connect(const std::string& host, int port);
    ssize_t send(const std::string& data);
    ssize_t receive(std::string& data, size_t size = 4096);
    void close();
    
    // Helper function
    static void initializeNetwork();
    static void cleanupNetwork();
    
    int getFD() const { return sockfd; }
};