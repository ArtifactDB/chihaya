cmake_minimum_required(VERSION 3.14)

project(chihaya
    VERSION 1.1.2
    DESCRIPTION "A C++ validator for delayed array operations"
    LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

add_library(chihaya INTERFACE)
add_library(artifactdb::chihaya ALIAS chihaya)

option(CHIHAYA_FIND_HDF5 "Try to find and link to HDF5 for chihaya." ON)
if(CHIHAYA_FIND_HDF5)
    find_package(HDF5 COMPONENTS C CXX)
    if (HDF5_FOUND)
        target_link_libraries(chihaya INTERFACE hdf5::hdf5 hdf5::hdf5_cpp)
    endif()
endif()

option(CHIHAYA_FETCH_EXTERN "Automatically fetch takane's external dependencies." ON)
if(CHIHAYA_FETCH_EXTERN)
    add_subdirectory(extern)
else()
    find_package(artifactdb_ritsuko 0.6.0 CONFIG REQUIRED)
endif()

target_link_libraries(chihaya INTERFACE artifactdb::ritsuko)

# Switch between include directories depending on whether the downstream is
# using the build directly or is using the installed package.
include(GNUInstallDirs)
target_include_directories(chihaya 
    INTERFACE 
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/artifactdb_chihaya>"
)

# Building the test-related machinery, if we are compiling this library directly.
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    option(CHIHAYA_TESTS "Build chihaya's test suite." ON)
else()
    option(CHIHAYA_TESTS "Build chihaya's test suite." OFF)
endif()

if(CHIHAYA_TESTS)
    include(CTest)
    if(BUILD_TESTING)
        add_subdirectory(tests)
    endif() 
endif()

# Installing for find_package.
include(CMakePackageConfigHelpers)

install(DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/artifactdb_chihaya)

install(TARGETS chihaya
    EXPORT chihayaTargets)

install(EXPORT chihayaTargets
    FILE artifactdb_chihayaTargets.cmake
    NAMESPACE artifactdb::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/artifactdb_chihaya)

configure_package_config_file(${CMAKE_CURRENT_SOURCE_DIR}/cmake/Config.cmake.in
    "${CMAKE_CURRENT_BINARY_DIR}/artifactdb_chihayaConfig.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/artifactdb_chihaya)

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/artifactdb_chihayaConfigVersion.cmake"
    COMPATIBILITY SameMajorVersion)

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/artifactdb_chihayaConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/artifactdb_chihayaConfigVersion.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/artifactdb_chihaya)
