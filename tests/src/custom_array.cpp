#include <gtest/gtest.h>
#include "chihaya/chihaya.hpp"
#include "utils.h"

TEST(CustomArray, Basic) {
    std::string path = "Test_custom.h5";

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        custom_array_opener(fhandle, "ext", { 50, 5, 10 }); 
    }
    {
        auto output = chihaya::validate(path, "ext"); 
        EXPECT_EQ(output.type, chihaya::FLOAT);

        const auto& dims = output.dimensions;
        EXPECT_EQ(dims.size(), 3);
        EXPECT_EQ(dims[0], 50);
        EXPECT_EQ(dims[1], 5);
        EXPECT_EQ(dims[2], 10);
    }

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        custom_array_opener(fhandle, "ext", { 17 }, "BOOLEAN"); 
    }
    {
        auto output = chihaya::validate(path, "ext"); 
        EXPECT_EQ(output.type, chihaya::BOOLEAN);
        const auto& dims = output.dimensions;
        EXPECT_EQ(dims.size(), 1);
        EXPECT_EQ(dims[0], 17);
    }

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        custom_array_opener(fhandle, "ext", { 20, 10 }, "STRING"); 
    }
    {
        auto output = chihaya::validate(path, "ext"); 
        EXPECT_EQ(output.type, chihaya::STRING);
        const auto& dims = output.dimensions;
        EXPECT_EQ(dims.size(), 2);
        EXPECT_EQ(dims[0], 20);
        EXPECT_EQ(dims[1], 10);
    }

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        custom_array_opener(fhandle, "ext", { 20, 17 }, "INTEGER"); 
    }
    {
        auto output = chihaya::validate(path, "ext"); 
        EXPECT_EQ(output.type, chihaya::INTEGER);
        const auto& dims = output.dimensions;
        EXPECT_EQ(dims.size(), 2);
        EXPECT_EQ(dims[0], 20);
        EXPECT_EQ(dims[1], 17);
    }
}

TEST(CustomArray, Errors) {
    std::string path = "Test_custom.h5";

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = custom_array_opener(fhandle, "ext", { 50, 5, 10 }); 
        ghandle.unlink("dimensions");
    }
    expect_error([&]() -> void { chihaya::validate(path, "ext"); }, "expected a dataset at 'dimensions'");

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        custom_array_opener(fhandle, "ext", { 50, -20 }); 
    }
    expect_error([&]() -> void { chihaya::validate(path, "ext"); }, "non-negative");

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = custom_array_opener(fhandle, "ext", { 50, -20 }); 
        ghandle.unlink("dimensions");

        hsize_t dims[2];
        dims[0] = 10;
        dims[1] = 10;
        H5::DataSpace dspace(2, dims);
        ghandle.createDataSet("dimensions", H5::PredType::NATIVE_INT, dspace);
    }
    expect_error([&]() -> void { chihaya::validate(path, "ext"); }, "1-dimensional");
    
    // Checking type.
    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = custom_array_opener(fhandle, "ext", { 50, 5, 10 }); 
        ghandle.unlink("type");
    }
    expect_error([&]() -> void { chihaya::validate(path, "ext"); }, "expected a dataset at 'type'");

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = custom_array_opener(fhandle, "ext", { 50, 5, 10 }); 
        ghandle.unlink("type");

        hsize_t dims = 10;
        H5::DataSpace dspace(1, &dims);
        ghandle.createDataSet("type", H5::PredType::NATIVE_INT, dspace);
    }
    expect_error([&]() -> void { chihaya::validate(path, "ext"); }, "'type' should be scalar");

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = custom_array_opener(fhandle, "ext", { 50, 5, 10 }, "FOOBAR"); 
    }
    expect_error([&]() -> void { chihaya::validate(path, "ext"); }, "(FOOBAR)");
}
