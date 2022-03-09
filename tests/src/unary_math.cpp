#include <gtest/gtest.h>
#include "chihaya/chihaya.hpp"
#include "utils.h"

TEST(UnaryMath, PureUnary) {
    std::string path = "Test_unary_math.h5";

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = operation_opener(fhandle, "hello", "unary math");
        external_array_opener(ghandle, "seed", { 13, 19 }, "INTEGER"); 
        add_scalar(ghandle, "method", std::string("abs"));
        add_scalar<double>(ghandle, "value", 2.5);
    }
    auto output = chihaya::validate(path, "hello"); 
    EXPECT_EQ(output.type, chihaya::INTEGER);

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = operation_opener(fhandle, "hello", "unary math");
        external_array_opener(ghandle, "seed", { 13, 19 }, "FLOAT"); 
        add_scalar(ghandle, "method", std::string("sign"));
    }
    output = chihaya::validate(path, "hello");
    EXPECT_EQ(output.type, chihaya::INTEGER);

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = operation_opener(fhandle, "hello", "unary math");
        external_array_opener(ghandle, "seed", { 13, 19 }, "INTEGER"); 
        add_scalar(ghandle, "method", std::string("log1p"));
    }
    output = chihaya::validate(path, "hello");
    EXPECT_EQ(output.type, chihaya::FLOAT);
}

TEST(UnaryMath, LogBase) {
    std::string path = "Test_unary_math.h5";

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = operation_opener(fhandle, "hello", "unary math");
        external_array_opener(ghandle, "seed", { 13, 19 }, "INTEGER"); 
        add_scalar(ghandle, "method", std::string("log"));
        add_scalar<double>(ghandle, "base", 2);
    }

    auto output = chihaya::validate(path, "hello"); 
    EXPECT_EQ(output.type, chihaya::FLOAT);
}

TEST(UnaryMath, RoundDigits) {
    std::string path = "Test_unary_math.h5";

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = operation_opener(fhandle, "hello", "unary math");
        external_array_opener(ghandle, "seed", { 5, 19 }, "FLOAT"); 
        add_scalar(ghandle, "method", std::string("round"));
        add_scalar(ghandle, "digits", 5);
    }

    auto output = chihaya::validate(path, "hello"); 
    EXPECT_EQ(output.type, chihaya::FLOAT);
}

TEST(UnaryMath, CommonErrors) {
    std::string path = "Test_unary_math.h5";

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        operation_opener(fhandle, "hello", "unary math");
    }
    expect_error([&]() -> void { chihaya::validate(path, "hello"); }, "expected 'seed'");

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = operation_opener(fhandle, "hello", "unary math");
        external_array_opener(ghandle, "seed", { 13, 19 }, "STRING"); 
    }
    expect_error([&]() -> void { chihaya::validate(path, "hello"); }, "should contain numeric or boolean");

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = operation_opener(fhandle, "hello", "unary math");
        external_array_opener(ghandle, "seed", { 13, 19 }, "INTEGER"); 
    }
    expect_error([&]() -> void { chihaya::validate(path, "hello"); }, "expected 'method'");

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = operation_opener(fhandle, "hello", "unary math");
        external_array_opener(ghandle, "seed", { 13, 19 }, "INTEGER"); 
        add_scalar<int>(ghandle, "method", 1);
    }
    expect_error([&]() -> void { chihaya::validate(path, "hello"); }, "scalar string");

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = operation_opener(fhandle, "hello", "unary math");
        external_array_opener(ghandle, "seed", { 13, 19 }, "INTEGER"); 
        add_scalar(ghandle, "method", std::string("foo"));
    }
    expect_error([&]() -> void { chihaya::validate(path, "hello"); }, "unrecognized 'method' (foo)");
}

TEST(UnaryMath, MethodErrors) {
    std::string path = "Test_unary_math.h5";

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = operation_opener(fhandle, "hello", "unary math");
        external_array_opener(ghandle, "seed", { 13, 19 }, "INTEGER"); 
        add_scalar(ghandle, "method", std::string("log"));
        ghandle.createGroup("base");
    }
    expect_error([&]() -> void { chihaya::validate(path, "hello"); }, "expected 'base'");

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = operation_opener(fhandle, "hello", "unary math");
        external_array_opener(ghandle, "seed", { 13, 19 }, "INTEGER"); 
        add_scalar(ghandle, "method", std::string("log"));
        add_scalar<int>(ghandle, "base", 2);
    }
    expect_error([&]() -> void { chihaya::validate(path, "hello"); }, "'base' should be a scalar float");

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = operation_opener(fhandle, "hello", "unary math");
        external_array_opener(ghandle, "seed", { 13, 19 }, "FLOAT"); 
        add_scalar(ghandle, "method", std::string("round"));
    }
    expect_error([&]() -> void { chihaya::validate(path, "hello"); }, "expected 'digits'");

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = operation_opener(fhandle, "hello", "unary math");
        external_array_opener(ghandle, "seed", { 13, 19 }, "FLOAT"); 
        add_scalar(ghandle, "method", std::string("round"));
        add_scalar<double>(ghandle, "digits", 2);
    }
    expect_error([&]() -> void { chihaya::validate(path, "hello"); }, "'digits' should be a scalar integer");
}
