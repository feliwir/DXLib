cmake_minimum_required(VERSION 3.24)
project(DXLIB LANGUAGES C CXX)

option(DXLIB_BUILD_TESTS "Build tests" ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(src)
if(DXLIB_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()