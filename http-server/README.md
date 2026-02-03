# HTTP Web Server

## Overview

**HTTP Web Server** is a lightweight, high-performance web server implementation that provides robust HTTP/1.1 protocol support. Designed with simplicity and efficiency in mind, this server efficiently handles client requests, serves static content, and maintains stable connections in concurrent environments.

## Features

- **Full HTTP/1.1 Compliance**: Supports standard HTTP methods including GET, POST, PUT, DELETE, and HEAD
- **Static File Serving**: Efficient delivery of HTML, CSS, JavaScript, images, and other static assets
- **Concurrent Connection Handling**: Multi-threaded/async architecture for handling multiple simultaneous clients
- **Configurable Routing**: Flexible URL path mapping with custom route handlers
- **MIME Type Detection**: Automatic content-type headers based on file extensions
- **Comprehensive Status Codes**: Proper HTTP response codes (200, 404, 500, etc.)
- **Connection Management**: Keep-alive support and proper connection lifecycle handling
- **Logging System**: Detailed request/response logging for debugging and monitoring
- **Configurable Settings**: Port configuration, root directory, timeout settings, and more

## Architecture

The server follows a modular architecture with clear separation of concerns:

1. **Socket Layer**: Manages network connections and TCP socket operations
2. **Protocol Parser**: Handles HTTP request parsing and response generation
3. **Request Handler**: Processes routes and serves appropriate content
4. **Static Resolver**: Maps URLs to filesystem resources
5. **Connection Pool**: Manages client connections and threading

## Installation

### Prerequisites
- C++17 compatible compiler (GCC, Clang, or MSVC)
- CMake 3.10 or higher

### Build Instructions
```bash
# Clone the repository
git clone https://github.com/yourusername/http-web-server.git
cd http-web-server

# Create build directory
mkdir build && cd build

# Configure and build
cmake ..
make

# Run the server
./webserver