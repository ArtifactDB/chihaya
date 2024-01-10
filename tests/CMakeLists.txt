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
    src/subset_assignment.cpp
    src/external_hdf5.cpp
    src/custom_array.cpp
    src/list.cpp
    src/combine.cpp
    src/transpose.cpp
    src/dimnames.cpp
    src/unary_arithmetic.cpp
    src/unary_comparison.cpp
    src/unary_logic.cpp
    src/unary_math.cpp
    src/unary_special_check.cpp
    src/binary_arithmetic.cpp
    src/binary_comparison.cpp
    src/binary_logic.cpp
    src/dense_array.cpp
    src/sparse_matrix.cpp
    src/matrix_product.cpp
    src/constant_array.cpp
)

target_link_libraries(
    libtest
    gtest_main
    chihaya
)

target_compile_options(libtest PRIVATE -Wall -Wextra -Wpedantic -Werror)

set(CODE_COVERAGE OFF CACHE BOOL "Enable coverage testing")
if(CODE_COVERAGE AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(libtest PRIVATE -O0 -g --coverage)
    target_link_options(libtest PRIVATE --coverage)
endif()

include(GoogleTest)
gtest_discover_tests(libtest)
