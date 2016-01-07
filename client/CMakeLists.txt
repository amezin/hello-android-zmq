cmake_minimum_required(VERSION 3.4)
project(hello-android-zmq-client)

find_package(cppzmq REQUIRED)

add_executable(client client.cpp)
target_link_libraries(client cppzmq)
