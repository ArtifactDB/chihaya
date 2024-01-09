#include <gtest/gtest.h>
#include "chihaya/chihaya.hpp"
#include "utils.h"

template<bool left>
void add_seed(H5::Group& parent, const std::vector<int>& dimensions, std::string type, bool transposed) {
    std::string thing = (left ? std::string("left") : std::string("right"));
    external_array_opener(parent, thing + "_seed", dimensions, type);
    add_scalar(parent, thing + "_orientation", (transposed ? std::string("T") : std::string("N")));
}

TEST(MatrixProduct, Simple) {
    std::string path = "Test_product.h5";

    // Both untransposed.
    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = operation_opener(fhandle, "foos", "matrix product");
        add_seed<true>(ghandle, { 10, 20 }, "FLOAT", false);
        add_seed<false>(ghandle, { 20, 15 }, "FLOAT", false);
    }
    {
        auto output = chihaya::validate(path, "foos"); 
        EXPECT_EQ(output.type, chihaya::FLOAT);
        EXPECT_EQ(output.dimensions[0], 10);
        EXPECT_EQ(output.dimensions[1], 15);
    }

    // One or the other transposed.
    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = operation_opener(fhandle, "foos", "matrix product");
        add_seed<true>(ghandle, { 10, 20 }, "FLOAT", true);
        add_seed<false>(ghandle, { 10, 15 }, "INTEGER", false);
    }
    {
        auto output = chihaya::validate(path, "foos"); 
        EXPECT_EQ(output.type, chihaya::FLOAT);
        EXPECT_EQ(output.dimensions[0], 20);
        EXPECT_EQ(output.dimensions[1], 15);
    }

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = operation_opener(fhandle, "foos", "matrix product");
        add_seed<true>(ghandle, { 10, 20 }, "INTEGER", false);
        add_seed<false>(ghandle, { 30, 20 }, "FLOAT", true);
    }
    {
        auto output = chihaya::validate(path, "foos"); 
        EXPECT_EQ(output.type, chihaya::FLOAT);
        EXPECT_EQ(output.dimensions[0], 10);
        EXPECT_EQ(output.dimensions[1], 30);
    }

    // Both transposed.
    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = operation_opener(fhandle, "foos", "matrix product");
        add_seed<true>(ghandle, { 20, 10 }, "INTEGER", true);
        add_seed<false>(ghandle, { 15, 20 }, "INTEGER", true);
    }
    {
        auto output = chihaya::validate(path, "foos"); 
        EXPECT_EQ(output.type, chihaya::INTEGER);
        EXPECT_EQ(output.dimensions[0], 10);
        EXPECT_EQ(output.dimensions[1], 15);
    }
}

TEST(MatrixProduct, Errors) {
    std::string path = "Test_product.h5";

    // Testing the seed properties.
    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = operation_opener(fhandle, "foos", "matrix product");
        add_seed<true>(ghandle, { 10, 20 }, "FLOAT", false);
    }
    expect_error([&]() -> void { chihaya::validate(path, "foos"); }, "expected a group at 'right_seed'");

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = operation_opener(fhandle, "foos", "matrix product");
        add_seed<true>(ghandle, { 10, 20 }, "FLOAT", false);
        external_array_opener(ghandle, "right_seed", { 10, 20, 30 });
    }
    expect_error([&]() -> void { chihaya::validate(path, "foos"); }, "2-dimensional");

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = operation_opener(fhandle, "foos", "matrix product");
        add_seed<true>(ghandle, { 10, 20 }, "FLOAT", false);
        external_array_opener(ghandle, "right_seed", { 10, 20 }, "STRING");
    }
    expect_error([&]() -> void { chihaya::validate(path, "foos"); }, "integer, float or boolean");

    // Checking the orientation.
    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = operation_opener(fhandle, "foos", "matrix product");
        add_seed<true>(ghandle, { 10, 20 }, "FLOAT", false);
        ghandle.unlink("left_orientation");
    }
    expect_error([&]() -> void { chihaya::validate(path, "foos"); }, "expected a dataset at 'left_orientation'");

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = operation_opener(fhandle, "foos", "matrix product");
        add_seed<true>(ghandle, { 10, 20 }, "FLOAT", false);
        ghandle.unlink("left_orientation");
        add_scalar(ghandle, "left_orientation", 10);
    }
    expect_error([&]() -> void { chihaya::validate(path, "foos"); }, "UTF-8 encoded string");

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = operation_opener(fhandle, "foos", "matrix product");
        add_seed<true>(ghandle, { 10, 20 }, "FLOAT", false);
        ghandle.unlink("left_orientation");
        add_scalar(ghandle, "left_orientation", std::string("FOO"));
    }
    expect_error([&]() -> void { chihaya::validate(path, "foos"); }, "'left_orientation' should be either 'N' or 'T'");

    // Checking the combination.
    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = operation_opener(fhandle, "foos", "matrix product");
        add_seed<true>(ghandle, { 10, 20 }, "FLOAT", false);
        add_seed<false>(ghandle, { 10, 15 }, "FLOAT", false);
    }
    expect_error([&]() -> void { chihaya::validate(path, "foos"); }, "inconsistent common dimensions");
}

