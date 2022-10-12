cmake_minimum_required(VERSION 3.23)
project(HTTPServer_Demo_CPP)

set(CMAKE_CXX_STANDARD 20)

add_executable(HTTPServer_Demo_CPP main.cpp)
if(WIN32)
    target_link_libraries(HTTPServer_Demo_CPP wsock32 ws2_32)
endif()
