cmake_minimum_required(VERSION 3.21)
project(api)

set(CMAKE_CXX_STANDARD 14)
link_libraries(ws2_32 wsock32)
add_executable(mongoose.h mongoose.c start.cpp)
