#include <gtest/gtest.h>
#include "chihaya/chihaya.hpp"
#include "utils.h"

class BinaryArithmeticTest : public ::testing::TestWithParam<int> {};

TEST_P(BinaryArithmeticTest, Simple) {
    auto version = GetParam();
    std::string path = "Test_binary_arithmetic.h5";

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = operation_opener(fhandle, "hello", "binary arithmetic");
        add_version_string(ghandle, version);

        CustomArrayOptions opt(version, "INTEGER");
        custom_array_opener(ghandle, "left", { 13, 19 }, opt); 
        custom_array_opener(ghandle, "right", { 13, 19 }, opt); 
        add_scalar(ghandle, "method", std::string("+"));
    }

    auto output = chihaya::validate(path, "hello"); 
    EXPECT_EQ(output.type, chihaya::INTEGER);
}

TEST_P(BinaryArithmeticTest, Promotion) {
    auto version = GetParam();
    std::string path = "Test_binary_arithmetic.h5";

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = operation_opener(fhandle, "hello", "binary arithmetic");
        add_version_string(ghandle, version);

        custom_array_opener(ghandle, "left", { 13, 19 }, CustomArrayOptions(version, "INTEGER"));
        custom_array_opener(ghandle, "right", { 13, 19 }, CustomArrayOptions(version, "FLOAT"));
        add_scalar(ghandle, "method", std::string("*"));
    }

    auto output = chihaya::validate(path, "hello"); 
    EXPECT_EQ(output.type, chihaya::FLOAT);
}

TEST_P(BinaryArithmeticTest, Errors) {
    auto version = GetParam();
    std::string path = "Test_binary_arithmetic.h5";

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = operation_opener(fhandle, "hello", "binary arithmetic");
        add_version_string(ghandle, version);
    }
    expect_error([&]() -> void { chihaya::validate(path, "hello"); }, "'left'");

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = operation_opener(fhandle, "hello", "binary arithmetic");
        add_version_string(ghandle, version);
        custom_array_opener(ghandle, "left", { 13, 19 }, CustomArrayOptions(version, "STRING"));
    }
    expect_error([&]() -> void { chihaya::validate(path, "hello"); }, "'left' should be");

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = operation_opener(fhandle, "hello", "binary arithmetic");
        add_version_string(ghandle, version);
        custom_array_opener(ghandle, "left", { 13, 19 }, CustomArrayOptions(version, "INTEGER")); 
    }
    expect_error([&]() -> void { chihaya::validate(path, "hello"); }, "'right'");

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = operation_opener(fhandle, "hello", "binary arithmetic");
        add_version_string(ghandle, version);

        CustomArrayOptions opt(version, "INTEGER");
        custom_array_opener(ghandle, "left", { 10, 5 }, opt);
        custom_array_opener(ghandle, "right", { 13, 19 }, opt);
    }
    expect_error([&]() -> void { chihaya::validate(path, "hello"); }, "'left' and 'right' should have the same");

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = operation_opener(fhandle, "hello", "binary arithmetic");
        add_version_string(ghandle, version);

        CustomArrayOptions opt(version, "INTEGER");
        custom_array_opener(ghandle, "left", { 13, 19 }, opt);
        custom_array_opener(ghandle, "right", { 13, 19 }, opt);
    }
    expect_error([&]() -> void { chihaya::validate(path, "hello"); }, "'method'");

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = operation_opener(fhandle, "hello", "binary arithmetic");
        add_version_string(ghandle, version);

        CustomArrayOptions opt(version, "INTEGER");
        custom_array_opener(ghandle, "left", { 13, 19 }, opt);
        custom_array_opener(ghandle, "right", { 13, 19 }, opt);
        add_scalar(ghandle, "method", std::string("foo"));
    }
    expect_error([&]() -> void { chihaya::validate(path, "hello"); }, "unrecognized 'method'");
}

INSTANTIATE_TEST_SUITE_P(
    BinaryArithmetic,
    BinaryArithmeticTest,
    ::testing::Values(0, 1000000, 1100000)
);
