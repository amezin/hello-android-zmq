include(FindPackageHandleStandardArgs)

if(NOT DEFINED ANDROID_SDK_SEARCH_PATHS)
 if(CMAKE_HOST_WIN32)
  file(TO_CMAKE_PATH "$ENV{PROGRAMFILES}" ANDROID_SDK_SEARCH_PATHS)
  set(ANDROID_SDK_SEARCH_PATHS "${ANDROID_SDK_SEARCH_PATHS}" "$ENV{SystemDrive}/NVPACK")
 else()
  file(TO_CMAKE_PATH "$ENV{HOME}" ANDROID_SDK_SEARCH_PATHS)
  set(ANDROID_SDK_SEARCH_PATHS /opt /usr/local/opt "${ANDROID_SDK_SEARCH_PATHS}/NVPACK")
 endif()
endif()

# Copy-pasted from OpenCVDetectAndroidSDK.cmake

if(CMAKE_HOST_WIN32)
  set(ANDROID_SDK_OS windows)
elseif(CMAKE_HOST_APPLE)
  set(ANDROID_SDK_OS macosx)
else()
  set(ANDROID_SDK_OS linux)
endif()

#find android SDK: search in ANDROID_SDK first
find_program(ANDROID_EXECUTABLE
  NAMES android.bat android
  PATH_SUFFIXES tools
  PATHS
    ENV ANDROID_SDK
  DOC "Android SDK location"
  NO_DEFAULT_PATH
  )

# Now search default paths
find_program(ANDROID_EXECUTABLE
  NAMES android.bat android
  PATH_SUFFIXES android-sdk-${ANDROID_SDK_OS}/tools
                android-sdk-${ANDROID_SDK_OS}_x86/tools
                android-sdk-${ANDROID_SDK_OS}_86/tools
                android-sdk/tools
  PATHS ${ANDROID_SDK_SEARCH_PATHS}
  DOC "Android SDK location"
  )

find_package_handle_standard_args(AndroidSDK DEFAULT_MSG ANDROID_EXECUTABLE)
mark_as_advanced(ANDROID_EXECUTABLE)
