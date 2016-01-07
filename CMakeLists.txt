cmake_minimum_required(VERSION 3.4)
project(hello-android-zmq NONE)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

list(INSERT CMAKE_MODULE_PATH 0 "${CMAKE_CURRENT_SOURCE_DIR}")
set(CMAKE_PREFIX_PATH "${CMAKE_CURRENT_BINARY_DIR}")

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/cppzmq-config.cmake.in"
               "${CMAKE_CURRENT_BINARY_DIR}/cppzmq-config.cmake"
               @ONLY)
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/cppzmq-config.cmake.in"
               "${CMAKE_CURRENT_BINARY_DIR}/android/cppzmq-config.cmake"
               @ONLY)

add_subdirectory(client)
add_subdirectory(android)
