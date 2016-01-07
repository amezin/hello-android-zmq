cmake_minimum_required(VERSION 3.4)
project(hello-android-zmq-server)

find_package(cppzmq REQUIRED)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(${PROJECT_NAME} SHARED activity.cpp launcher.cpp util.hpp)
target_link_libraries(${PROJECT_NAME} android log cppzmq)
install(TARGETS ${PROJECT_NAME} DESTINATION apk/libs/${ANDROID_ABI})
