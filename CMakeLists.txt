cmake_minimum_required(VERSION 3.15)
# set the project name
project(MySerializer)
add_definitions(-std=c++17)

include_directories(include)

# link to include/thirdparty/*.cpp
file(GLOB_RECURSE THIRD_PARTY_SOURCES "include/thirdparty/*.cpp")
add_library(thirdparty STATIC ${THIRD_PARTY_SOURCES})

set(EXECUTABLE_OUTPUT_PATH "tests")
add_executable(test_binary tests/test_binary.cpp)
add_executable(test_xml tests/test_xml.cpp include/thirdparty/tinyxml2.cpp)
target_link_libraries(test_xml thirdparty)
add_executable(test_xml_b64 tests/test_xml_b64.cpp)
target_link_libraries(test_xml_b64 thirdparty)
