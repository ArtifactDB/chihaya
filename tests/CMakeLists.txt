include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/c9461a9b55ba954df0489bab6420eb297bed846b.zip
)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

# Main test executable.
add_executable(
    libtest 
    src/subset.cpp
    src/external.cpp
    src/list.cpp
)

target_link_libraries(
    libtest
    gtest_main
    chihaya
)

find_package(HDF5 REQUIRED COMPONENTS C CXX)
target_link_libraries(libtest hdf5::hdf5 hdf5::hdf5_cpp)

include(GoogleTest)
gtest_discover_tests(libtest)

