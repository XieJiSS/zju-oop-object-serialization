cmake_minimum_required(VERSION 3.15)
# set the project name
project(MySerializer)
add_definitions(-std=c++17)

include_directories(include)

add_executable(test_binary test-binary.cpp)
