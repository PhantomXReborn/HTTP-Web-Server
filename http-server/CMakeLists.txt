cmake_minimum_required(VERSION 3.10)
project(HTTPServer VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# For Windows, link Winsock
if(WIN32)
    add_definitions(-D_WIN32_WINNT=0x0A00)
    set(PLATFORM_LIBS ws2_32)
else()
    set(PLATFORM_LIBS pthread)
endif()

add_executable(httpserver
    src/main.cpp
    src/server/Server.cpp
    src/socket/Socket.cpp
    src/http/Request.cpp
    src/http/Response.cpp
    src/utils/FileHandler.cpp
    src/utils/Logger.cpp
    src/config/Config.cpp
)

target_link_libraries(httpserver ${PLATFORM_LIBS})