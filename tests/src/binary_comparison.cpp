#include <gtest/gtest.h>

#include <string>
#include <vector>
#include <cstddef>

#include "H5Cpp.h"
#include "chihaya/binary_comparison.hpp"

#include "utils.h"

static H5::Group binary_comparison_opener(H5::Group& handle, const std::string& name, const std::string& method, const ritsuko::Version& version) {
    auto ghandle = operation_opener(handle, name, "binary comparison");
    add_version_string(ghandle, version);
    add_string_scalar(ghandle, "method", method);
    return ghandle;
}

/***********************************/

class BinaryComparisonPassTest : public ::testing::TestWithParam<std::tuple<ritsuko::Version, bool> > {};

TEST_P(BinaryComparisonPassTest, Simple) {
    auto path = define_test_path("binary_comparison");
    auto params = GetParam();
    auto version = std::get<0>(params);
    auto deets = std::get<1>(params);

    std::vector<std::size_t> dimensions{ 13, 19 };
    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = binary_comparison_opener(fhandle, "hello", "==", version);
        mock_array_opener(ghandle, "left", dimensions, version, "INTEGER");
        mock_array_opener(ghandle, "right", dimensions, version, "INTEGER");
    }
    {
        auto output = test_validate(path, "hello", deets); 
        EXPECT_EQ(output.type, chihaya::BOOLEAN);
        EXPECT_EQ(output.dimensions, dimensions);
    }

    // Works with two strings as well.
    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = binary_comparison_opener(fhandle, "hello", ">", version);
        mock_array_opener(ghandle, "left", dimensions, version, "STRING");
        mock_array_opener(ghandle, "right", dimensions, version, "STRING");
    }
    {
        auto output = test_validate(path, "hello", deets); 
        EXPECT_EQ(output.type, chihaya::BOOLEAN);
        EXPECT_EQ(output.dimensions, dimensions);
    }
}

TEST_P(BinaryComparisonPassTest, Mixed) {
    auto path = define_test_path("binary_comparison");
    auto params = GetParam();
    auto version = std::get<0>(params);
    auto deets = std::get<1>(params);

    std::vector<std::size_t> dimensions{ 52, 34 };
    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = binary_comparison_opener(fhandle, "hello", ">", version);
        mock_array_opener(ghandle, "left", dimensions, version, "INTEGER");
        mock_array_opener(ghandle, "right", dimensions, version, "FLOAT");
    }
    {
        auto output = test_validate(path, "hello", deets); 
        EXPECT_EQ(output.type, chihaya::BOOLEAN);
        EXPECT_EQ(output.dimensions, dimensions);
    }

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = binary_comparison_opener(fhandle, "hello", ">", version);
        mock_array_opener(ghandle, "left", dimensions, version, "INTEGER");
        mock_array_opener(ghandle, "right", dimensions, version, "BOOLEAN");
    }
    {
        auto output = test_validate(path, "hello", deets);
        EXPECT_EQ(output.type, chihaya::BOOLEAN);
        EXPECT_EQ(output.dimensions, dimensions);
    }
}

INSTANTIATE_TEST_SUITE_P(
    BinaryComparison,
    BinaryComparisonPassTest,
    ::testing::Combine(
        spawn_all_versions(),
        ::testing::Values(false, true)
    )
);

/***********************************/

class BinaryComparisonErrorTest : public ::testing::TestWithParam<ritsuko::Version> {};

TEST_P(BinaryComparisonErrorTest, Seed) {
    auto path = define_test_path("binary_comparison");
    auto version = GetParam();

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = binary_comparison_opener(fhandle, "hello", ">", version);
        mock_array_opener<int>(ghandle, "left", { 13, 19 }, version, "STRING");
        mock_array_opener<int>(ghandle, "right", { 13, 19 }, version, "INTEGER");
    }
    expect_error(path, "hello", "both or neither");

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = binary_comparison_opener(fhandle, "hello", ">", version);
        mock_array_opener<int>(ghandle, "left", { 13, 19 }, version, "STRING");
        mock_array_opener<int>(ghandle, "right", { 13, 19 }, version, "INTEGER");
    }
    expect_error(path, "hello", "both or neither");

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = binary_comparison_opener(fhandle, "hello", ">", version);
        mock_array_opener<int>(ghandle, "left", { 10, 5 }, version, "INTEGER"); 
        mock_array_opener<int>(ghandle, "right", { 13, 19 }, version, "INTEGER"); 
    }
    expect_error(path, "hello", "'left' and 'right' should have the same");
}

TEST_P(BinaryComparisonErrorTest, Method) {
    auto path = define_test_path("binary_comparison");
    auto version = GetParam();

    // Test that we actually check for a scalar and string 'method'.
    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = binary_comparison_opener(fhandle, "hello", "==", version);
        mock_array_opener<int>(ghandle, "left", { 13, 19 }, version, "BOOLEAN");
        mock_array_opener<int>(ghandle, "right", { 13, 19 }, version, "BOOLEAN");
        ghandle.unlink("method");
        add_numeric_vector<int>(ghandle, "method", { 5 }, H5::PredType::NATIVE_INT);
    }
    expect_error(path, "hello", "scalar");

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = binary_comparison_opener(fhandle, "hello", "foo", version);
        mock_array_opener<int>(ghandle, "left", { 13, 19 }, version, "INTEGER");
        mock_array_opener<int>(ghandle, "right", { 13, 19 }, version, "INTEGER"); 
    }
    expect_error(path, "hello", "unrecognized 'method'");
}

INSTANTIATE_TEST_SUITE_P(
    BinaryComparison,
    BinaryComparisonErrorTest,
    spawn_all_versions()
);
