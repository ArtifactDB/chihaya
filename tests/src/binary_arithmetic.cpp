#include <gtest/gtest.h>
#include "chihaya/chihaya.hpp"
#include "utils.h"

class BinaryArithmeticTest : public ::testing::TestWithParam<int> {
public:
    BinaryArithmeticTest() : path("Test_binary_arithmetic.h5") {}
protected:
    std::string path;

    static H5::Group binary_arithmetic_opener(H5::Group& handle, const std::string& name, const std::string& method, int version) {
        auto ghandle = operation_opener(handle, name, "binary arithmetic");
        add_version_string(ghandle, version);
        add_string_scalar(ghandle, "method", method);
        return ghandle;
    }
};

TEST_P(BinaryArithmeticTest, Simple) {
    auto version = GetParam();

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = binary_arithmetic_opener(fhandle, "hello", "+", version);
        mock_array_opener(ghandle, "left", { 13, 19 }, version, "INTEGER"); 
        mock_array_opener(ghandle, "right", { 13, 19 }, version, "INTEGER"); 
    }

    auto output = test_validate(path, "hello"); 
    EXPECT_EQ(output.type, chihaya::INTEGER);
}

TEST_P(BinaryArithmeticTest, Promotion) {
    auto version = GetParam();

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = binary_arithmetic_opener(fhandle, "hello", "*", version);
        mock_array_opener(ghandle, "left", { 13, 19 }, version, "INTEGER");
        mock_array_opener(ghandle, "right", { 13, 19 }, version, "FLOAT");
    }

    auto output = test_validate(path, "hello"); 
    EXPECT_EQ(output.type, chihaya::FLOAT);
}

TEST_P(BinaryArithmeticTest, Errors) {
    auto version = GetParam();

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = binary_arithmetic_opener(fhandle, "hello", "*", version);
    }
    expect_error(path, "hello", "'left'");

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = binary_arithmetic_opener(fhandle, "hello", "*", version);
        mock_array_opener(ghandle, "left", { 13, 19 }, version, "STRING");
    }
    expect_error(path, "hello", "'left' should be");

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = binary_arithmetic_opener(fhandle, "hello", "*", version);
        mock_array_opener(ghandle, "left", { 13, 19 }, version, "INTEGER"); 
    }
    expect_error(path, "hello", "'right'");

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = binary_arithmetic_opener(fhandle, "hello", "*", version);
        mock_array_opener(ghandle, "left", { 10, 5 }, version, "INTEGER");
        mock_array_opener(ghandle, "right", { 13, 19 }, version, "INTEGER");
    }
    expect_error(path, "hello", "'left' and 'right' should have the same");
}

TEST_P(BinaryArithmeticTest, MethodErrors) {
    auto version = GetParam();

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = binary_arithmetic_opener(fhandle, "hello", "*", version);
        mock_array_opener(ghandle, "left", { 13, 19 }, version, "INTEGER");
        mock_array_opener(ghandle, "right", { 13, 19 }, version, "INTEGER");
        ghandle.unlink("method");
    }
    expect_error(path, "hello", "'method'");

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = binary_arithmetic_opener(fhandle, "hello", "foo", version);
        mock_array_opener(ghandle, "left", { 13, 19 }, version, "INTEGER");
        mock_array_opener(ghandle, "right", { 13, 19 }, version, "INTEGER");
    }
    expect_error(path, "hello", "unrecognized 'method'");
}

INSTANTIATE_TEST_SUITE_P(
    BinaryArithmetic,
    BinaryArithmeticTest,
    ::testing::Values(0, 1000000, 1100000)
);
