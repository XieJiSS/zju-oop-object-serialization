cmake_minimum_required(VERSION 3.15)
# set the project name
project(MySerializer)
add_definitions(-std=c++17)

include_directories(include)

set(EXECUTABLE_OUTPUT_PATH "tests")
add_executable(test_binary tests/test_binary.cpp)
add_executable(test_xml tests/test_xml.cpp include/thirdparty/tinyxml2.cpp)
add_executable(test_xml_b64 tests/test_xml_b64.cpp include/thirdparty/tinyxml2.cpp include/thirdparty/base64.cpp)
