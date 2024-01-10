#include <gtest/gtest.h>
#include "chihaya/chihaya.hpp"
#include "utils.h"

class UnaryMathTest : public ::testing::TestWithParam<int> {
public:
    UnaryMathTest() : path("Test_unary_math.h5") {}
protected:
    std::string path;

    static H5::Group unary_math_opener(H5::Group& handle, const std::string& name, const std::string& method, const std::vector<int>& dimensions, int version, const std::string& type) {
        auto ghandle = operation_opener(handle, name, "unary math");
        add_version_string(ghandle, version);
        mock_array_opener(ghandle, "seed", dimensions, version, type);
        add_string_scalar(ghandle, "method", method);
        return ghandle;
    }
};

TEST_P(UnaryMathTest, PureUnary) {
    auto version = GetParam();

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        unary_math_opener(fhandle, "hello", "abs", { 13, 19 }, version, "INTEGER");
    }
    auto output = test_validate(path, "hello"); 
    EXPECT_EQ(output.type, chihaya::INTEGER);
    EXPECT_EQ(output.dimensions.size(), 2);
    EXPECT_EQ(output.dimensions[0], 13);
    EXPECT_EQ(output.dimensions[1], 19);

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        unary_math_opener(fhandle, "hello", "sign", { 13, 19 }, version, "FLOAT");
    }
    output = test_validate(path, "hello");
    EXPECT_EQ(output.type, chihaya::INTEGER);

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        unary_math_opener(fhandle, "hello", "log1p", { 13, 19 }, version, "BOOLEAN");
    }
    output = test_validate(path, "hello");
    EXPECT_EQ(output.type, chihaya::FLOAT);
}

TEST_P(UnaryMathTest, LogBase) {
    auto version = GetParam();

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = unary_math_opener(fhandle, "hello", "log", { 14, 23 }, version, "INTEGER");
        add_numeric_scalar<double>(ghandle, "base", 2, H5::PredType::NATIVE_DOUBLE);
    }

    auto output = test_validate(path, "hello"); 
    EXPECT_EQ(output.type, chihaya::FLOAT);
    EXPECT_EQ(output.dimensions.size(), 2);
    EXPECT_EQ(output.dimensions[0], 14);
    EXPECT_EQ(output.dimensions[1], 23);
}

TEST_P(UnaryMathTest, RoundDigits) {
    auto version = GetParam();

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = unary_math_opener(fhandle, "hello", "round", { 5, 12 }, version, "FLOAT");
        add_numeric_scalar<int>(ghandle, "digits", 2, H5::PredType::NATIVE_INT32);
    }

    auto output = test_validate(path, "hello"); 
    EXPECT_EQ(output.type, chihaya::FLOAT);
    EXPECT_EQ(output.dimensions.size(), 2);
    EXPECT_EQ(output.dimensions[0], 5);
    EXPECT_EQ(output.dimensions[1], 12);
}

TEST_P(UnaryMathTest, SeedErrors) {
    auto version = GetParam();

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        unary_math_opener(fhandle, "hello", "round", { 5, 12 }, version, "STRING");
    }
    expect_error(path, "hello", "should be integer, float or boolean");

    {
        H5::H5File fhandle(path, H5F_ACC_RDWR);
        auto ghandle = fhandle.openGroup("hello");
        ghandle.unlink("seed");
    }
    expect_error(path, "hello", "expected a group at 'seed'");
}

TEST_P(UnaryMathTest, MethodErrors) {
    auto version = GetParam();

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = unary_math_opener(fhandle, "hello", "sin", { 5, 12 }, version, "FLOAT");
        ghandle.unlink("method");
    }
    expect_error(path, "hello", "expected a dataset at 'method'");

    {
        H5::H5File fhandle(path, H5F_ACC_RDWR);
        auto ghandle = fhandle.openGroup("hello");
        add_numeric_scalar<int>(ghandle, "method", 1, H5::PredType::NATIVE_INT);
    }
    expect_error(path, "hello", "UTF-8 encoded string");

    {
        H5::H5File fhandle(path, H5F_ACC_RDWR);
        auto ghandle = fhandle.openGroup("hello");
        ghandle.unlink("method");
        add_string_scalar(ghandle, "method", "foo");
    }
    expect_error(path, "hello", "unrecognized operation");
}

TEST_P(UnaryMathTest, OtherErrors) {
    auto version = GetParam();

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = unary_math_opener(fhandle, "hello", "log", { 5, 12 }, version, "FLOAT");
        ghandle.createGroup("base");
    }
    expect_error(path, "hello", "expected 'base'");

    {
        H5::H5File fhandle(path, H5F_ACC_RDWR);
        auto ghandle = fhandle.openGroup("hello");
        ghandle.unlink("base");
        add_string_scalar(ghandle, "base", "foo");
    }
    if (version < 1100000) {
        expect_error(path, "hello", "'base' should be a float");
    } else {
        expect_error(path, "hello", "64-bit float");
    }

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        unary_math_opener(fhandle, "hello", "signif", { 5, 12 }, version, "FLOAT");
    }
    expect_error(path, "hello", "expected a dataset at 'digits'");

    {
        H5::H5File fhandle(path, H5F_ACC_RDWR);
        auto ghandle = fhandle.openGroup("hello");
        add_numeric_scalar<double>(ghandle, "digits", 2, H5::PredType::NATIVE_DOUBLE);
    }
    if (version < 1100000) {
        expect_error(path, "hello", "'digits' should be an integer");
    } else {
        expect_error(path, "hello", "32-bit signed integer");
    }
}

INSTANTIATE_TEST_SUITE_P(
    UnaryMath,
    UnaryMathTest,
    ::testing::Values(0, 1000000, 1100000)
);
