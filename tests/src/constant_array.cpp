#include <gtest/gtest.h>
#include "chihaya/chihaya.hpp"
#include "utils.h"

template<class T>
H5::Group constant_array_opener(H5::Group& handle, std::string name, const std::vector<int>& dimensions, T type) {
    auto ghandle = array_opener(handle, name, "constant array");
    add_vector<int>(ghandle, "dimensions", dimensions);
    add_scalar(ghandle, "value", type);
    return ghandle;
}

TEST(Constant, Basic) {
    std::string path = "Test_constant_array.h5";

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        constant_array_opener(fhandle, "constant", { 20, 17 }, 1);
    }
    {
        auto output = chihaya::validate(path, "constant"); 
        EXPECT_EQ(output.type, chihaya::INTEGER);
        const auto& dims = output.dimensions;
        EXPECT_EQ(dims.size(), 2);
        EXPECT_EQ(dims[0], 20);
        EXPECT_EQ(dims[1], 17);
    }

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        constant_array_opener(fhandle, "constant", { 5, 17 }, 2.0);
    }
    {
        auto output = chihaya::validate(path, "constant"); 
        EXPECT_EQ(output.type, chihaya::FLOAT);
        const auto& dims = output.dimensions;
        EXPECT_EQ(dims.size(), 2);
        EXPECT_EQ(dims[0], 5);
        EXPECT_EQ(dims[1], 17);
    }

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        constant_array_opener(fhandle, "constant", { 20, 17 }, 1);
        add_version_string(fhandle.openGroup("constant"), "1.0.0");
        auto dhandle = fhandle.openDataSet("constant/value");
        add_missing_placeholder(dhandle, (int)1);
    }
    {
        auto output = chihaya::validate(path, "constant"); 
        EXPECT_EQ(output.type, chihaya::INTEGER);
    }
}

TEST(Constant, Errors) {
    std::string path = "Test_constant_array.h5";

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = constant_array_opener(fhandle, "constant", { 20, 17 }, 2.0);
        ghandle.unlink("dimensions");
        add_vector<double>(ghandle, "dimensions", {20, 17});
    }
    expect_error([&]() -> void { chihaya::validate(path, "constant"); }, "'dimensions' should be integer");

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = constant_array_opener(fhandle, "constant", {}, 2.0);
    }
    expect_error([&]() -> void { chihaya::validate(path, "constant"); }, "should have non-zero");

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = constant_array_opener(fhandle, "constant", { -1 }, 2.0);
    }
    expect_error([&]() -> void { chihaya::validate(path, "constant"); }, "non-negative");

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = constant_array_opener(fhandle, "constant", { 50, 10 }, 1);
        ghandle.unlink("value");
        ghandle.createGroup("value");
    }
    expect_error([&]() -> void { chihaya::validate(path, "constant"); }, "expected a dataset at 'value'");

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = constant_array_opener(fhandle, "constant", { 50, 10 }, 1);
        ghandle.unlink("value");
        add_vector<int>(ghandle, "value", { 20, 20 });
    }
    expect_error([&]() -> void { chihaya::validate(path, "constant"); }, "should be a scalar");

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        constant_array_opener(fhandle, "constant", { 20, 17 }, 1);
        add_version_string(fhandle.openGroup("constant"), "1.0.0");
        auto dhandle = fhandle.openDataSet("constant/value");
        add_missing_placeholder(dhandle, 1.0);
    }
    expect_error([&]() -> void { chihaya::validate(path, "constant"); }, "have the same type class");

}

