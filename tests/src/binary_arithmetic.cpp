#include <gtest/gtest.h>
#include "chihaya/chihaya.hpp"
#include "utils.h"

TEST(BinaryArithmetic, Simple) {
    std::string path = "Test_binary_arithmetic.h5";

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = operation_opener(fhandle, "hello", "binary arithmetic");
        external_array_opener(ghandle, "left", { 13, 19 }, "INTEGER"); 
        external_array_opener(ghandle, "right", { 13, 19 }, "INTEGER"); 
        add_scalar(ghandle, "method", std::string("+"));
    }

    auto output = chihaya::validate(path, "hello"); 
    EXPECT_EQ(output.type, chihaya::INTEGER);
}

TEST(BinaryArithmetic, Promotion) {
    std::string path = "Test_binary_arithmetic.h5";

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = operation_opener(fhandle, "hello", "binary arithmetic");
        external_array_opener(ghandle, "left", { 13, 19 }, "INTEGER"); 
        external_array_opener(ghandle, "right", { 13, 19 }, "FLOAT"); 
        add_scalar(ghandle, "method", std::string("*"));
    }

    auto output = chihaya::validate(path, "hello"); 
    EXPECT_EQ(output.type, chihaya::FLOAT);
}

TEST(BinaryArithmetic, Errors) {
    std::string path = "Test_binary_arithmetic.h5";

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        operation_opener(fhandle, "hello", "binary arithmetic");
    }
    expect_error([&]() -> void { chihaya::validate(path, "hello"); }, "'left'");

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = operation_opener(fhandle, "hello", "binary arithmetic");
        external_array_opener(ghandle, "left", { 13, 19 }, "STRING"); 
    }
    expect_error([&]() -> void { chihaya::validate(path, "hello"); }, "'left' should be");

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = operation_opener(fhandle, "hello", "binary arithmetic");
        external_array_opener(ghandle, "left", { 13, 19 }, "INTEGER"); 
    }
    expect_error([&]() -> void { chihaya::validate(path, "hello"); }, "'right'");

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = operation_opener(fhandle, "hello", "binary arithmetic");
        external_array_opener(ghandle, "left", { 10, 5 }, "INTEGER"); 
        external_array_opener(ghandle, "right", { 13, 19 }, "INTEGER"); 
    }
    expect_error([&]() -> void { chihaya::validate(path, "hello"); }, "'left' and 'right' should have the same");

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = operation_opener(fhandle, "hello", "binary arithmetic");
        external_array_opener(ghandle, "left", { 13, 19 }, "INTEGER"); 
        external_array_opener(ghandle, "right", { 13, 19 }, "INTEGER"); 
    }
    expect_error([&]() -> void { chihaya::validate(path, "hello"); }, "'method'");

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = operation_opener(fhandle, "hello", "binary arithmetic");
        external_array_opener(ghandle, "left", { 13, 19 }, "INTEGER"); 
        external_array_opener(ghandle, "right", { 13, 19 }, "INTEGER"); 
        add_scalar(ghandle, "method", std::string("foo"));
    }
    expect_error([&]() -> void { chihaya::validate(path, "hello"); }, "unrecognized 'method'");
}
