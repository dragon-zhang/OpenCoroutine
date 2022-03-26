cmake_minimum_required(VERSION 3.17)
project(open-coroutine-native)

set(CMAKE_CXX_STANDARD 14)

include_directories(src/fiber)

add_library(jni-lib SHARED
        src/native_checker_api_impl.cpp
        src/fiber_api_impl.cpp
        src/scheduler_api_impl.cpp
        src/system_call_hook_api_impl.cpp)

include_directories(head)
IF (WIN32)
    include_directories(head/windows)
ELSEIF (APPLE)
    include_directories(head/macos)
ELSEIF (common/linux)
    include_directories(head/linux)
ENDIF ()