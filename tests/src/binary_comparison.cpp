#include <gtest/gtest.h>
#include "chihaya/chihaya.hpp"
#include "utils.h"

TEST(BinaryComparison, Simple) {
    std::string path = "Test_binary_comparison.h5";

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = operation_opener(fhandle, "hello", "binary comparison");
        external_array_opener(ghandle, "left", { 13, 19 }, "INTEGER"); 
        external_array_opener(ghandle, "right", { 13, 19 }, "INTEGER"); 
        add_scalar(ghandle, "method", std::string("=="));
    }
    auto output = chihaya::validate(path, "hello"); 
    EXPECT_EQ(output.type, chihaya::BOOLEAN);
}

TEST(BinaryComparison, Mixed) {
    std::string path = "Test_binary_comparison.h5";

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = operation_opener(fhandle, "hello", "binary comparison");
        external_array_opener(ghandle, "left", { 13, 19 }, "INTEGER"); 
        external_array_opener(ghandle, "right", { 13, 19 }, "FLOAT"); 
        add_scalar(ghandle, "method", std::string(">"));
    }
    auto output = chihaya::validate(path, "hello"); 
    EXPECT_EQ(output.type, chihaya::BOOLEAN);

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = operation_opener(fhandle, "hello", "binary comparison");
        external_array_opener(ghandle, "left", { 13, 19 }, "STRING"); 
        external_array_opener(ghandle, "right", { 13, 19 }, "STRING"); 
        add_scalar(ghandle, "method", std::string(">"));
    }
    output = chihaya::validate(path, "hello"); 
    EXPECT_EQ(output.type, chihaya::BOOLEAN);
}

TEST(BinaryComparison, Errors) {
    std::string path = "Test_binary_comparison.h5";

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        operation_opener(fhandle, "hello", "binary comparison");
    }
    expect_error([&]() -> void { chihaya::validate(path, "hello"); }, "expected 'left'");

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = operation_opener(fhandle, "hello", "binary comparison");
        external_array_opener(ghandle, "left", { 13, 19 }, "INTEGER"); 
    }
    expect_error([&]() -> void { chihaya::validate(path, "hello"); }, "expected 'right'");

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = operation_opener(fhandle, "hello", "binary comparison");
        external_array_opener(ghandle, "left", { 13, 19 }, "STRING"); 
        external_array_opener(ghandle, "right", { 13, 19 }, "INTEGER"); 
    }
    expect_error([&]() -> void { chihaya::validate(path, "hello"); }, "both or none");

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = operation_opener(fhandle, "hello", "binary comparison");
        external_array_opener(ghandle, "left", { 10, 5 }, "INTEGER"); 
        external_array_opener(ghandle, "right", { 13, 19 }, "INTEGER"); 
    }
    expect_error([&]() -> void { chihaya::validate(path, "hello"); }, "'left' and 'right' should have the same");

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = operation_opener(fhandle, "hello", "binary comparison");
        external_array_opener(ghandle, "left", { 13, 19 }, "INTEGER"); 
        external_array_opener(ghandle, "right", { 13, 19 }, "INTEGER"); 
    }
    expect_error([&]() -> void { chihaya::validate(path, "hello"); }, "expected 'method'");

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = operation_opener(fhandle, "hello", "binary comparison");
        external_array_opener(ghandle, "left", { 13, 19 }, "INTEGER"); 
        external_array_opener(ghandle, "right", { 13, 19 }, "INTEGER"); 
        add_scalar(ghandle, "method", std::string("foo"));
    }
    expect_error([&]() -> void { chihaya::validate(path, "hello"); }, "unrecognized 'method'");
}
