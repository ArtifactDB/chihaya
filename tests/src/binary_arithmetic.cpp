#include <gtest/gtest.h>

#include <string>
#include <vector>
#include <cstddef>

#include "H5Cpp.h"
#include "chihaya/binary_arithmetic.hpp"

#include "utils.h"

static H5::Group binary_arithmetic_opener(H5::Group& handle, const std::string& name, const std::string& method, const ritsuko::Version& version) {
    auto ghandle = operation_opener(handle, name, "binary arithmetic");
    add_version_string(ghandle, version);
    add_string_scalar(ghandle, "method", method);
    return ghandle;
}

/***********************************/

class BinaryArithmeticPassTest : public ::testing::TestWithParam<std::tuple<ritsuko::Version, bool> > {};

TEST_P(BinaryArithmeticPassTest, Simple) {
    auto path = define_test_path("binary_arithmetic");
    auto params = GetParam();
    auto version = std::get<0>(params);
    auto deets = std::get<1>(params);

    std::vector<std::size_t> dimensions{ 13, 19 };
    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = binary_arithmetic_opener(fhandle, "hello", "+", version);
        mock_array_opener(ghandle, "left", dimensions, version, "INTEGER"); 
        mock_array_opener(ghandle, "right", dimensions, version, "INTEGER"); 
    }

    auto output = test_validate(path, "hello", deets);
    EXPECT_EQ(output.type, chihaya::INTEGER);
    EXPECT_EQ(output.dimensions, dimensions);
}

TEST_P(BinaryArithmeticPassTest, Promotion) {
    auto path = define_test_path("binary_arithmetic");
    auto params = GetParam();
    auto version = std::get<0>(params);
    auto deets = std::get<1>(params);

    std::vector<std::size_t> dimensions{ 50, 32 };
    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = binary_arithmetic_opener(fhandle, "hello", "*", version);
        mock_array_opener(ghandle, "left", dimensions, version, "INTEGER");
        mock_array_opener(ghandle, "right", dimensions, version, "FLOAT");
    }

    auto output = test_validate(path, "hello", deets); 
    EXPECT_EQ(output.type, chihaya::FLOAT);
    EXPECT_EQ(output.dimensions, dimensions);
}

INSTANTIATE_TEST_SUITE_P(
    BinaryArithmetic,
    BinaryArithmeticPassTest,
    ::testing::Combine(
        spawn_all_versions(),
        ::testing::Values(false, true)
    )
);

/***********************************/

class BinaryArithmeticErrorTest : public ::testing::TestWithParam<ritsuko::Version> {};

TEST_P(BinaryArithmeticErrorTest, Seed) {
    auto path = define_test_path("binary_arithmetic");
    auto version = GetParam();

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = binary_arithmetic_opener(fhandle, "hello", "*", version);
        mock_array_opener<int>(ghandle, "left", { 13, 19 }, version, "STRING");
    }
    expect_error(path, "hello", "'left' should be");

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = binary_arithmetic_opener(fhandle, "hello", "*", version);
        mock_array_opener<int>(ghandle, "left", { 13, 19 }, version, "INTEGER");
        mock_array_opener<int>(ghandle, "right", { 13, 19 }, version, "STRING");
    }
    expect_error(path, "hello", "'right' should be");

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = binary_arithmetic_opener(fhandle, "hello", "*", version);
        mock_array_opener<int>(ghandle, "left", { 10, 5 }, version, "INTEGER");
        mock_array_opener<int>(ghandle, "right", { 13, 19 }, version, "INTEGER");
    }
    expect_error(path, "hello", "'left' and 'right' should have the same");
}

TEST_P(BinaryArithmeticErrorTest, Method) {
    auto path = define_test_path("binary_arithmetic");
    auto version = GetParam();

    // Test that we actually check for a scalar and string 'method'.
    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = binary_arithmetic_opener(fhandle, "hello", "+", version);
        mock_array_opener<int>(ghandle, "left", { 13, 19 }, version, "INTEGER");
        mock_array_opener<int>(ghandle, "right", { 13, 19 }, version, "INTEGER");
        ghandle.unlink("method");
        add_numeric_vector<int>(ghandle, "method", { 5 }, H5::PredType::NATIVE_INT);
    }
    expect_error(path, "hello", "scalar");

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = binary_arithmetic_opener(fhandle, "hello", "foo", version);
        mock_array_opener<int>(ghandle, "left", { 13, 19 }, version, "INTEGER");
        mock_array_opener<int>(ghandle, "right", { 13, 19 }, version, "INTEGER");
    }
    expect_error(path, "hello", "unrecognized 'method'");
}

INSTANTIATE_TEST_SUITE_P(
    BinaryArithmetic,
    BinaryArithmeticErrorTest,
    spawn_all_versions()
);
