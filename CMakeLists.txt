cmake_minimum_required(VERSION 3.15)
# set the project name
project(MySerializer)
add_definitions(-std=c++17)

include_directories(include)

add_executable(test_binary test_binary.cpp)
add_executable(test_xml test_xml.cpp include/thirdparty/tinyxml2.cpp)
add_executable(test_xml_b64 test_xml_b64.cpp include/thirdparty/tinyxml2.cpp include/thirdparty/base64.cpp)
