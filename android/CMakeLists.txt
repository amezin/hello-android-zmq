list(INSERT CMAKE_MODULE_PATH 0 "${CMAKE_CURRENT_SOURCE_DIR}/android-cmake")

find_package(AndroidSDK REQUIRED)

if(NOT ANDROID_API_LEVEL)
    set(ANDROID_API_LEVEL 19)
endif()
if(NOT ANDROID_TARGET)
    set(ANDROID_TARGET android-${ANDROID_API_LEVEL})
endif()
if(NOT ANDROID_BUILD_TYPE)
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(ANDROID_BUILD_TYPE Debug)
    else()
        set(ANDROID_BUILD_TYPE Release)
    endif()
endif()

set(ANDROID_COMMON_ARGS
    -DCMAKE_MODULE_PATH:PATH=${CMAKE_MODULE_PATH}
    -DANDROID_NATIVE_API_LEVEL:STRING=${ANDROID_TARGET}
    -DCMAKE_BUILD_TYPE:STRING=${ANDROID_BUILD_TYPE}
    -DCMAKE_TOOLCHAIN_FILE:FILEPATH=${CMAKE_CURRENT_SOURCE_DIR}/android-cmake/android.toolchain.cmake)

unset(BUILT_TARGETS)

include(ExternalProject)

foreach(ANDROID_ABI armeabi-v7a x86)
    set(ANDROID_ABI_ARGS ${ANDROID_COMMON_ARGS} -DANDROID_ABI:STRING=${ANDROID_ABI})

    ExternalProject_Add(ZeroMQ-android-${ANDROID_ABI}
                        URL "https://github.com/zeromq/zeromq4-1/archive/v4.1.4.zip"
                        CMAKE_CACHE_ARGS
                        ${ANDROID_ABI_ARGS}
                        -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_CURRENT_BINARY_DIR}/${ANDROID_ABI}
                        -DZMQ_BUILD_TESTS:BOOL=OFF
                        CMAKE_CACHE_DEFAULT_ARGS
                        -DZMQ_HAVE_SO_KEEPALIVE_EXITCODE:INTERNAL=0
                        -DZMQ_HAVE_TCP_KEEPIDLE_EXITCODE:INTERNAL=0
                        -DZMQ_HAVE_TCP_KEEPINTVL_EXITCODE:INTERNAL=0
                        -DZMQ_HAVE_TCP_KEEPCNT_EXITCODE:INTERNAL=0)

    ExternalProject_Add(server-${ANDROID_ABI}
                        DEPENDS ZeroMQ-android-${ANDROID_ABI}
                        SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/server"
                        CMAKE_CACHE_ARGS
                        ${ANDROID_ABI_ARGS}
                        -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_CURRENT_BINARY_DIR}
                        -DCMAKE_PREFIX_PATH:PATH=/${ANDROID_ABI}
                        -DZeroMQ_FIND_STATIC:BOOL=ON
                        BUILD_ALWAYS 1)

    list(APPEND BUILT_TARGETS server-${ANDROID_ABI})
endforeach()

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/AndroidManifest.xml.in"
               "${CMAKE_CURRENT_BINARY_DIR}/apk/AndroidManifest.xml")

add_custom_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/apk/build.xml"
                   COMMAND ${ANDROID_EXECUTABLE} update project
                   --target ${ANDROID_TARGET}
                   --path ${CMAKE_CURRENT_BINARY_DIR}/apk
                   --name HelloAndroidZMQ
                   DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/apk/AndroidManifest.xml)
add_custom_target(android-project ALL
                  DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/apk/build.xml")

find_package(Ant)
if(ANT_FOUND)
    if(NOT APK_BUILD_TYPE)
        string(TOLOWER "${ANDROID_BUILD_TYPE}" APK_BUILD_TYPE)
    endif()
    add_custom_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/apk/bin/HelloAndroidZMQ-${APK_BUILD_TYPE}.apk"
                       COMMAND ${ANT_EXECUTABLE} ${APK_BUILD_TYPE}
                       DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/apk/build.xml" "${BUILT_TARGETS}"
                       WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/apk")
    add_custom_target(apk ALL
                      DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/apk/bin/HelloAndroidZMQ-${APK_BUILD_TYPE}.apk")
    install(CODE "execute_process(COMMAND \"${ANT_EXECUTABLE}\" \"${APK_BUILD_TYPE}\" install WORKING_DIRECTORY \"${CMAKE_CURRENT_BINARY_DIR}/apk\")")
endif()
