cmake_minimum_required(VERSION 3.4)
project(hello-android-zmq-server)

find_package(cppzmq REQUIRED)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(AndroidNdkModules)
android_ndk_import_module_native_app_glue()

add_library(${PROJECT_NAME} SHARED server.cpp)
target_link_libraries(${PROJECT_NAME} android native_app_glue cppzmq)
install(TARGETS ${PROJECT_NAME} DESTINATION apk/libs/${ANDROID_ABI})
