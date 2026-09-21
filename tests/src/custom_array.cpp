#include <gtest/gtest.h>

#include <string>
#include <vector>
#include <cstddef>

#include "H5Cpp.h"
#include "ritsuko/ritsuko.hpp"
#include "chihaya/custom_array.hpp"

#include "utils.h"

static H5::Group custom_array_opener(H5::Group& handle, const std::string& name, const std::vector<std::size_t>& dimensions, const ritsuko::Version& version, const std::string& type) {
    auto ghandle = array_opener(handle, name, "custom thingy");
    add_version_string(ghandle, version);
    add_string_scalar(ghandle, "type", type);

    if (version.lt(1, 1, 0)) {
        add_numeric_vector(ghandle, "dimensions", dimensions, H5::PredType::NATIVE_INT);
    } else {
        add_numeric_vector(ghandle, "dimensions", dimensions, H5::PredType::NATIVE_UINT32);
    }
    return ghandle;
}

/***********************************/

class CustomArrayPassTest : public ::testing::TestWithParam<std::tuple<ritsuko::Version, bool> > {};

TEST_P(CustomArrayPassTest, Float) {
    auto path = define_test_path("custom_array");
    auto params = GetParam();
    auto version = std::get<0>(params);
    auto deets = std::get<1>(params);

    std::vector<std::size_t> dimensions{ 50, 5, 10 };
    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        custom_array_opener(fhandle, "ext", dimensions, version, "FLOAT");
    }

    auto output = test_validate(path, "ext", deets);
    EXPECT_EQ(output.type, chihaya::FLOAT);
    EXPECT_EQ(output.dimensions, dimensions);
}

TEST_P(CustomArrayPassTest, Boolean) {
    auto path = define_test_path("custom_array");
    auto params = GetParam();
    auto version = std::get<0>(params);
    auto deets = std::get<1>(params);

    std::vector<std::size_t> dimensions{ 17 };
    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        custom_array_opener(fhandle, "ext", dimensions, version, "BOOLEAN");
    }

    auto output = test_validate(path, "ext", deets);
    EXPECT_EQ(output.type, chihaya::BOOLEAN);
    EXPECT_EQ(output.dimensions, dimensions);
}

TEST_P(CustomArrayPassTest, String) {
    auto path = define_test_path("custom_array");
    auto params = GetParam();
    auto version = std::get<0>(params);
    auto deets = std::get<1>(params);

    std::vector<std::size_t> dimensions{ 23, 72 };
    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        custom_array_opener(fhandle, "ext", dimensions, version, "STRING");
    }

    auto output = test_validate(path, "ext", deets);
    EXPECT_EQ(output.type, chihaya::STRING);
    EXPECT_EQ(output.dimensions, dimensions);
}

TEST_P(CustomArrayPassTest, Integer) {
    auto path = define_test_path("custom_array");
    auto params = GetParam();
    auto version = std::get<0>(params);
    auto deets = std::get<1>(params);

    std::vector<std::size_t> dimensions{ 37, 17 };
    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        custom_array_opener(fhandle, "ext", dimensions, version, "INTEGER");
    }

    auto output = test_validate(path, "ext", deets); 
    EXPECT_EQ(output.type, chihaya::INTEGER);
    EXPECT_EQ(output.dimensions, dimensions);
}

INSTANTIATE_TEST_SUITE_P(
    CustomArray,
    CustomArrayPassTest,
    ::testing::Combine(
        spawn_all_versions(),
        ::testing::Values(false, true)
    )
);

/***********************************/

class CustomArrayErrorTest : public ::testing::TestWithParam<ritsuko::Version> {};

TEST_P(CustomArrayErrorTest, Dimension) {
    auto path = define_test_path("custom_array");
    auto version = GetParam();

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = custom_array_opener(fhandle, "ext", {}, version, "INTEGER");
        ghandle.unlink("dimensions");
        std::vector<hsize_t> dims(2);
        H5::DataSpace dspace(2, dims.data());
        ghandle.createDataSet("dimensions", H5::PredType::NATIVE_UINT32, dspace);
    }
    expect_error(path, "ext", "1-dimensional");

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = custom_array_opener(fhandle, "ext", {}, version, "INTEGER");
        ghandle.unlink("dimensions");
        add_numeric_vector<int>(ghandle, "dimensions", { 50, 20 }, H5::PredType::NATIVE_DOUBLE);
    }
    if (version.lt(1, 1, 0)) {
        expect_error(path, "ext", "expected an integer type");
    } else {
        expect_error(path, "ext", "64-bit unsigned integer");
    }

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = custom_array_opener(fhandle, "ext", { 50, 12 }, version, "INTEGER");
        ghandle.unlink("dimensions");
        add_numeric_vector<int>(ghandle, "dimensions", { 50, -20 }, H5::PredType::NATIVE_INT);
    }
    if (version.lt(1, 1, 0)) {
        expect_error(path, "ext", "non-negative");
    } else {
        expect_error(path, "ext", "64-bit unsigned integer");
    }
}

TEST_P(CustomArrayErrorTest, Type) {
    auto path = define_test_path("custom_array");
    auto version = GetParam();

    // Test that we check for a scalar string dataset.
    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = custom_array_opener(fhandle, "ext", { 50, 5, 10 }, version, "INTEGER"); 
        ghandle.unlink("type");
        add_string_vector(ghandle, "type", 10, /* strlen = */ 5);
    }
    expect_error(path, "ext", "should be scalar");

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = custom_array_opener(fhandle, "ext", { 50, 5, 10 }, version, "FOOBAR");
    }
    expect_error(path, "ext", "(FOOBAR)");
}

INSTANTIATE_TEST_SUITE_P(
    CustomArray,
    CustomArrayErrorTest,
    spawn_all_versions()
);
