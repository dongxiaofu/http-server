cmake_minimum_required(VERSION 3.16)
project(http-server)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
#set(SOURCE_FILES httpServer.cpp httpServer.cpp)

#include_directories(SYSTEM "/foo/bar")

add_executable(http-server
        httpServer.cpp
        Request.cpp
        Request.h
        fpm/Common.h
        fpm/Common.cpp
        fpm/FcgiRole.cpp
        fpm/FcgiRequestType.cpp
        fpm/Fcgi.cpp
        fpm/Fcgi.h
        fpm/Fpm.cpp
        fpm/Fpm.h)

add_executable(test demo/cout_demo.cpp)