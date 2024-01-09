#include <gtest/gtest.h>
#include "chihaya/chihaya.hpp"
#include "utils.h"

TEST(Transpose, NoOp) {
    std::string path = "Test_transpose.h5";

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = operation_opener(fhandle, "hello", "transpose");
        external_array_opener(ghandle, "seed", { 13, 19 }); 
        add_vector<int>(ghandle, "permutation", { 0, 1 });
    }

    auto output = chihaya::validate(path, "hello"); 
    const auto& dims = output.dimensions;
    EXPECT_EQ(dims[0], 13);
    EXPECT_EQ(dims[1], 19);
}

TEST(Transpose, Simple) {
    std::string path = "Test_transpose.h5";

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = operation_opener(fhandle, "hello", "transpose");
        external_array_opener(ghandle, "seed", { 13, 19 }); 
        add_vector<int>(ghandle, "permutation", { 1, 0 });
    }

    auto output = chihaya::validate(path, "hello"); 
    const auto& dims = output.dimensions;
    EXPECT_EQ(dims[0], 19);
    EXPECT_EQ(dims[1], 13);
}

TEST(Transpose, Complex) {
    std::string path = "Test_transpose.h5";

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = operation_opener(fhandle, "hello", "transpose");
        external_array_opener(ghandle, "seed", { 13, 19, 5 }); 
        add_vector<int>(ghandle, "permutation", { 1, 2, 0 });
    }

    auto output = chihaya::validate(path, "hello"); 
    const auto& dims = output.dimensions;
    EXPECT_EQ(dims[0], 19);
    EXPECT_EQ(dims[1], 5);
    EXPECT_EQ(dims[2], 13);
}

TEST(Transpose, Errors) {
    std::string path = "Test_transpose.h5";

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        operation_opener(fhandle, "hello", "transpose");
    }
    expect_error([&]() -> void { chihaya::validate(path, "hello"); }, "expected a group at 'seed'");

    {
        H5::H5File fhandle(path, H5F_ACC_RDWR);
        auto ghandle = fhandle.openGroup("hello");
        external_array_opener(ghandle, "seed", { 13, 19 }); 
    }
    expect_error([&]() -> void { chihaya::validate(path, "hello"); }, "expected a dataset at 'permutation'");

    {
        H5::H5File fhandle(path, H5F_ACC_RDWR);
        auto ghandle = fhandle.openGroup("hello");
        add_vector<double>(ghandle, "permutation", { 1, 2, 0 });
    }
    expect_error([&]() -> void { chihaya::validate(path, "hello"); }, "'permutation' should be a 1-dimensional integer");

    {
        H5::H5File fhandle(path, H5F_ACC_RDWR);
        auto ghandle = fhandle.openGroup("hello");
        ghandle.unlink("permutation");
        add_vector<int>(ghandle, "permutation", { 1, 2, 0 });
    }
    expect_error([&]() -> void { chihaya::validate(path, "hello"); }, "length of 'permutation'");

    {
        H5::H5File fhandle(path, H5F_ACC_RDWR);
        auto ghandle = fhandle.openGroup("hello");
        ghandle.unlink("permutation");
        add_vector<int>(ghandle, "permutation", { 1, 5 });
    }
    expect_error([&]() -> void { chihaya::validate(path, "hello"); }, "out-of-bounds");

    {
        H5::H5File fhandle(path, H5F_ACC_RDWR);
        auto ghandle = fhandle.openGroup("hello");
        ghandle.unlink("permutation");
        add_vector<int>(ghandle, "permutation", { 0, 0 });
    }
    expect_error([&]() -> void { chihaya::validate(path, "hello"); }, "unique");
}
