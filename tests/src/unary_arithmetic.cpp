#include <gtest/gtest.h>
#include "chihaya/chihaya.hpp"
#include "utils.h"

TEST(UnaryArithmetic, PureUnary) {
    std::string path = "Test_unary_arithmetic.h5";

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = operation_opener(fhandle, "hello", "unary arithmetic");
        external_array_opener(ghandle, "seed", { 13, 19 }, "INTEGER"); 
        add_scalar(ghandle, "method", std::string("+"));
        add_scalar(ghandle, "side", std::string("none"));
    }

    auto output = chihaya::validate(path, "hello"); 
    EXPECT_EQ(output.type, chihaya::INTEGER);
}

TEST(UnaryArithmetic, ScalarUnary) {
    std::string path = "Test_unary_arithmetic.h5";

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = operation_opener(fhandle, "hello", "unary arithmetic");
        external_array_opener(ghandle, "seed", { 13, 19 }, "INTEGER"); 
        add_scalar(ghandle, "method", std::string("-"));
        add_scalar(ghandle, "side", std::string("left"));
        add_scalar<double>(ghandle, "value", 2.5);
    }

    auto output = chihaya::validate(path, "hello"); 
    EXPECT_EQ(output.type, chihaya::FLOAT);
}

TEST(UnaryArithmetic, VectorUnary) {
    std::string path = "Test_unary_arithmetic.h5";

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = operation_opener(fhandle, "hello", "unary arithmetic");
        external_array_opener(ghandle, "seed", { 5, 19 }, "INTEGER"); 
        add_scalar(ghandle, "method", std::string("/"));
        add_scalar(ghandle, "side", std::string("left"));
        add_scalar(ghandle, "along", 0);
        add_vector<double>(ghandle, "value", { 1, 2, 3, 4, 5 });
    }

    auto output = chihaya::validate(path, "hello"); 
    EXPECT_EQ(output.type, chihaya::FLOAT);
}

TEST(UnaryArithmetic, CommonErrors) {
    std::string path = "Test_unary_arithmetic.h5";

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        operation_opener(fhandle, "hello", "unary arithmetic");
    }
    expect_error([&]() -> void { chihaya::validate(path, "hello"); }, "expected 'seed'");

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = operation_opener(fhandle, "hello", "unary arithmetic");
        external_array_opener(ghandle, "seed", { 13, 19 }, "INTEGER"); 
    }
    expect_error([&]() -> void { chihaya::validate(path, "hello"); }, "expected 'method'");

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = operation_opener(fhandle, "hello", "unary arithmetic");
        external_array_opener(ghandle, "seed", { 13, 19 }, "INTEGER"); 
        add_scalar<int>(ghandle, "method", 1);
    }
    expect_error([&]() -> void { chihaya::validate(path, "hello"); }, "scalar string");

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = operation_opener(fhandle, "hello", "unary arithmetic");
        external_array_opener(ghandle, "seed", { 13, 19 }, "INTEGER"); 
        add_scalar(ghandle, "method", std::string("foo"));
    }
    expect_error([&]() -> void { chihaya::validate(path, "hello"); }, "unrecognized 'method' (foo)");

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = operation_opener(fhandle, "hello", "unary arithmetic");
        external_array_opener(ghandle, "seed", { 13, 19 }, "INTEGER"); 
        add_scalar(ghandle, "method", std::string("+"));
    }
    expect_error([&]() -> void { chihaya::validate(path, "hello"); }, "expected 'side'");

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = operation_opener(fhandle, "hello", "unary arithmetic");
        external_array_opener(ghandle, "seed", { 13, 19 }, "INTEGER"); 
        add_scalar(ghandle, "method", std::string("+"));
        add_scalar<int>(ghandle, "side", 1);
    }
    expect_error([&]() -> void { chihaya::validate(path, "hello"); }, "scalar string");

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = operation_opener(fhandle, "hello", "unary arithmetic");
        external_array_opener(ghandle, "seed", { 13, 19 }, "INTEGER"); 
        add_scalar(ghandle, "method", std::string("+"));
        add_scalar(ghandle, "side", std::string("foo"));
    }
    expect_error([&]() -> void { chihaya::validate(path, "hello"); }, "unrecognized 'side' (foo)");
}

TEST(UnaryArithmetic, MethodErrors) {
    std::string path = "Test_unary_arithmetic.h5";

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = operation_opener(fhandle, "hello", "unary arithmetic");
        external_array_opener(ghandle, "seed", { 13, 19 }, "INTEGER"); 
        add_scalar(ghandle, "method", std::string("/"));
        add_scalar(ghandle, "side", std::string("none"));
    }
    expect_error([&]() -> void { chihaya::validate(path, "hello"); }, "cannot be 'none'");

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = operation_opener(fhandle, "hello", "unary arithmetic");
        external_array_opener(ghandle, "seed", { 13, 19 }, "INTEGER"); 
        add_scalar(ghandle, "method", std::string("/"));
        add_scalar(ghandle, "side", std::string("left"));
    }
    expect_error([&]() -> void { chihaya::validate(path, "hello"); }, "expected 'value'");

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = operation_opener(fhandle, "hello", "unary arithmetic");
        external_array_opener(ghandle, "seed", { 13, 19 }, "INTEGER"); 
        add_scalar(ghandle, "method", std::string("/"));
        add_scalar(ghandle, "side", std::string("left"));
        add_scalar(ghandle, "value", std::string("WHEE"));
    }
    expect_error([&]() -> void { chihaya::validate(path, "hello"); }, "'value' dataset should be numeric");

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = operation_opener(fhandle, "hello", "unary arithmetic");
        external_array_opener(ghandle, "seed", { 13, 19 }, "INTEGER"); 
        add_scalar(ghandle, "method", std::string("/"));
        add_scalar(ghandle, "side", std::string("left"));

        hsize_t dims[2];
        dims[0] = 5;
        dims[1] = 5;
        H5::DataSpace dspace(2, dims);
        ghandle.createDataSet("value", H5::PredType::NATIVE_INT, dspace);
    }
    expect_error([&]() -> void { chihaya::validate(path, "hello"); }, "'value' dataset should be scalar or 1-dimensional");
}

TEST(UnaryArithmetic, AlongErrors) {
    std::string path = "Test_unary_arithmetic.h5";

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = operation_opener(fhandle, "hello", "unary arithmetic");
        external_array_opener(ghandle, "seed", { 19, 4 }, "INTEGER"); 
        add_scalar(ghandle, "method", std::string("/"));
        add_scalar(ghandle, "side", std::string("left"));
        add_vector<int>(ghandle, "value", { 1, 2, 3, 4 });
    }
    expect_error([&]() -> void { chihaya::validate(path, "hello"); }, "expected 'along'");

    {
        H5::H5File fhandle(path, H5F_ACC_RDWR);
        auto ghandle = fhandle.openGroup("hello");
        add_scalar(ghandle, "along", std::string("WHEE"));
    }
    expect_error([&]() -> void { chihaya::validate(path, "hello"); }, "'along' should be a scalar");

    {
        H5::H5File fhandle(path, H5F_ACC_RDWR);
        auto ghandle = fhandle.openGroup("hello");
        ghandle.unlink("along");
        add_scalar<int>(ghandle, "along", -1);
    }
    expect_error([&]() -> void { chihaya::validate(path, "hello"); }, "'along' should be non-negative");

    {
        H5::H5File fhandle(path, H5F_ACC_RDWR);
        auto ghandle = fhandle.openGroup("hello");
        ghandle.unlink("along");
        add_scalar<int>(ghandle, "along", 0);
    }
    expect_error([&]() -> void { chihaya::validate(path, "hello"); }, "length of 'value' dataset");
}