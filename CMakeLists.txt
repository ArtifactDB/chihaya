cmake_minimum_required(VERSION 3.14)

project(chihaya
    VERSION 1.0.0
    DESCRIPTION "A C++ validator for delayed array operations"
    LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

add_library(chihaya INTERFACE)

target_include_directories(chihaya INTERFACE include)

# Building the test-related machinery, if we are compiling this library directly.
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    option(INSTALL_GTEST OFF)
    include(CTest)
    if(BUILD_TESTING)
        add_subdirectory(tests)
    endif() 
endif()
