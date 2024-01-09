#include <gtest/gtest.h>
#include "chihaya/chihaya.hpp"
#include "utils.h"

class CustomArrayTest : public ::testing::TestWithParam<int> {
public:
    CustomArrayTest() : path("Test_custom.h5") {}
protected:
    std::string path;

    static H5::Group custom_array_opener(H5::Group& handle, const std::string& name, const std::vector<int>& dimensions, int version, std::string type) {
        auto ghandle = array_opener(handle, name, "custom thingy");
        add_version_string(ghandle, version);
        add_string_scalar(ghandle, "type", type);

        if (version < 1100000) {
            add_numeric_vector(ghandle, "dimensions", dimensions, H5::PredType::NATIVE_INT);
        } else {
            add_numeric_vector(ghandle, "dimensions", dimensions, H5::PredType::NATIVE_UINT32);
        }
        return ghandle;
    }
};

TEST_P(CustomArrayTest, Basic) {
    auto version = GetParam();

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        custom_array_opener(fhandle, "ext", { 50, 5, 10 }, version, "FLOAT");
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
        custom_array_opener(fhandle, "ext", { 17 }, version, "BOOLEAN");
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
        custom_array_opener(fhandle, "ext", { 20, 10 }, version, "STRING");
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
        custom_array_opener(fhandle, "ext", { 20, 17 }, version, "INTEGER");
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

TEST_P(CustomArrayTest, Errors) {
    auto version = GetParam();

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = custom_array_opener(fhandle, "ext", { 99, 12 }, version, "INTEGER");
        ghandle.unlink("dimensions");
    }
    expect_error([&]() -> void { chihaya::validate(path, "ext"); }, "expected a dataset at 'dimensions'");

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = custom_array_opener(fhandle, "ext", { 50, 12 }, version, "INTEGER");
        ghandle.unlink("dimensions");
        add_numeric_vector<int>(ghandle, "dimensions", { 50, -20 }, H5::PredType::NATIVE_DOUBLE);
    }
    if (version < 1100000) {
        expect_error([&]() -> void { chihaya::validate(path, "ext"); }, "should be integer");
    } else {
        expect_error([&]() -> void { chihaya::validate(path, "ext"); }, "64-bit unsigned integer");
    }

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = custom_array_opener(fhandle, "ext", { 50, 12 }, version, "INTEGER");
        ghandle.unlink("dimensions");
        add_numeric_vector<int>(ghandle, "dimensions", { 50, -20 }, H5::PredType::NATIVE_INT);
    }
    if (version < 1100000) {
        expect_error([&]() -> void { chihaya::validate(path, "ext"); }, "non-negative");
    } else {
        expect_error([&]() -> void { chihaya::validate(path, "ext"); }, "64-bit unsigned integer");
    }

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = custom_array_opener(fhandle, "ext", { 50, 10 }, version, "INTEGER"); 
        ghandle.unlink("dimensions");

        hsize_t dims[2];
        dims[0] = 10;
        dims[1] = 10;
        H5::DataSpace dspace(2, dims);
        ghandle.createDataSet("dimensions", H5::PredType::NATIVE_UINT32, dspace);
    }
    expect_error([&]() -> void { chihaya::validate(path, "ext"); }, "1-dimensional");
}

TEST_P(CustomArrayTest, TypeErrors) {
    auto version = GetParam();

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = custom_array_opener(fhandle, "ext", { 50, 5, 10 }, version, "INTEGER"); 
        ghandle.unlink("type");
    }
    expect_error([&]() -> void { chihaya::validate(path, "ext"); }, "expected a dataset at 'type'");

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = custom_array_opener(fhandle, "ext", { 50, 12 }, version, "INTEGER");
        ghandle.unlink("type");
        add_string_vector(ghandle, "type", 10);
    }
    expect_error([&]() -> void { chihaya::validate(path, "ext"); }, "'type' should be scalar");

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = custom_array_opener(fhandle, "ext", { 50, 5, 10 }, version, "FOOBAR");
    }
    expect_error([&]() -> void { chihaya::validate(path, "ext"); }, "(FOOBAR)");
}

INSTANTIATE_TEST_SUITE_P(
    CustomArray,
    CustomArrayTest,
    ::testing::Values(0, 1000000, 1100000)
);
