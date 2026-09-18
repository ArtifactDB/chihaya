#include <gtest/gtest.h>

#include <cstddef>
#include <vector>
#include <string>

#include "chihaya/chihaya.hpp"
#include "utils.h"

static H5::Group dimnames_opener(H5::Group& handle, const std::string& name, const std::vector<std::size_t>& dims, const std::string& type, int version) {
    auto ghandle = operation_opener(handle, name, "dimnames");
    add_version_string(ghandle, version);
    mock_array_opener(ghandle, "seed", dims, version, type);
    list_opener(ghandle, "dimnames", dims.size(), version);
    return ghandle;
}

/***********************************/

class DimnamesPassTest : public ::testing::TestWithParam<std::tuple<int, bool> > {};

TEST_P(DimnamesTest, Full) {
    auto version = GetParam();

    std::vector<std::size_t> dimensions{ 12, 20 };
    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = dimnames_opener(fhandle, "hello", dimensions, "INTEGER", version);
        auto lhandle = ghandle.openGroup("dimnames");
        add_string_vector(lhandle, "0", 12, /* len = */ 5);
        add_string_vector(lhandle, "1", 20, /* len = */ 2);
    }

    auto output = test_validate(path, "hello", deets); 
    EXPECT_EQ(output.type, chihaya::INTEGER);
    EXPECT_EQ(output.dimensions, dimensions);
}

INSTANTIATE_TEST_SUITE_P(
    Dimnames,
    DimnamesTest,
    ::testing::Values(0, 1000000, 1100000)
);

/***********************************/

TEST_P(DimnamesTest, Errors) {
    auto version = GetParam();

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = dimnames_opener(fhandle, "hello", { 0, 0 }, "INTEGER", version);
        ghandle.unlink("dimnames");
        list_opener(ghandle, "dimnames", 3, version);
    }
    expect_error(path, "hello", "length of 'dimnames' list");

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = dimnames_opener(fhandle, "hello", { 10, 20 }, "INTEGER", version);
        auto lhandle = ghandle.openGroup("dimnames");
        add_numeric_vector<int>(lhandle, "0", {1}, H5::PredType::NATIVE_INT32);
    }
    expect_error(path, "hello", "UTF-8 strings");

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = dimnames_opener(fhandle, "hello", { 10, 20 }, "INTEGER", version);
        auto lhandle = ghandle.openGroup("dimnames");
        add_string_vector(lhandle, "1", 15, /* len = */ 3);
    }
    expect_error(path, "hello", "length equal to the extent");

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = dimnames_opener(fhandle, "hello", { 10, 20 }, "INTEGER", version);
        auto lhandle = ghandle.openGroup("dimnames");
        add_string_vector(lhandle, "1", 20, /* len = */ H5T_VARIABLE);
    }
    expect_error(path, "hello", "NULL");
}

INSTANTIATE_TEST_SUITE_P(
    Dimnames,
    DimnamesTest,
    ::testing::Values(0, 1000000, 1100000)
);
