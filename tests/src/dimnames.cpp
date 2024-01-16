#include <gtest/gtest.h>
#include "chihaya/chihaya.hpp"
#include "utils.h"

class DimnamesTest : public ::testing::TestWithParam<int> {
public:
    DimnamesTest() : path("Test_dimnames.h5") {}
protected:
    std::string path;

    static H5::Group dimnames_opener(H5::Group& handle, const std::string& name, const std::vector<int>& dims, const std::string& type, int version) {
        auto ghandle = operation_opener(handle, name, "dimnames");
        add_version_string(ghandle, version);
        mock_array_opener(ghandle, "seed", dims, version, type);
        list_opener(ghandle, "dimnames", dims.size(), version);
        return ghandle;
    }
};

TEST_P(DimnamesTest, NoOp) {
    auto version = GetParam();

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        dimnames_opener(fhandle, "hello", { 13, 19 }, "FLOAT", version);
    }

    auto output = test_validate(path, "hello"); 
    EXPECT_EQ(output.type, chihaya::FLOAT);
    const auto& dims = output.dimensions;
    EXPECT_EQ(dims[0], 13);
    EXPECT_EQ(dims[1], 19);

    auto skipped = test_validate_skip(path, "hello");
    EXPECT_EQ(skipped.type, output.type);
    EXPECT_EQ(skipped.dimensions, output.dimensions);
}

TEST_P(DimnamesTest, Partial) {
    auto version = GetParam();

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = dimnames_opener(fhandle, "hello", { 13, 10 }, "INTEGER", version);
        auto lhandle = ghandle.openGroup("dimnames");
        add_string_vector(lhandle, "1", 10, /* len = */ 5);
    }

    auto output = test_validate(path, "hello"); 
    EXPECT_EQ(output.type, chihaya::INTEGER);
    const auto& dims = output.dimensions;
    EXPECT_EQ(dims[0], 13);
    EXPECT_EQ(dims[1], 10);
}

TEST_P(DimnamesTest, Full) {
    auto version = GetParam();

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = dimnames_opener(fhandle, "hello", { 13, 10 }, "INTEGER", version);
        auto lhandle = ghandle.openGroup("dimnames");
        add_string_vector(lhandle, "0", 13, /* len = */ 5);
        add_string_vector(lhandle, "1", 10, /* len = */ 2);
    }

    auto output = test_validate(path, "hello"); 
    EXPECT_EQ(output.type, chihaya::INTEGER);
    const auto& dims = output.dimensions;
    EXPECT_EQ(dims[0], 13);
    EXPECT_EQ(dims[1], 10);
}

TEST_P(DimnamesTest, Errors) {
    auto version = GetParam();

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = dimnames_opener(fhandle, "hello", { 0, 0 }, "INTEGER", version);
        ghandle.unlink("seed");
    }
    expect_error(path, "hello", "expected a group at 'seed'");

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = dimnames_opener(fhandle, "hello", { 0, 0 }, "INTEGER", version);
        ghandle.unlink("dimnames");
    }
    expect_error(path, "hello", "expected a 'dimnames' group");

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = dimnames_opener(fhandle, "hello", { 0, 0 }, "INTEGER", version);
        auto lhandle = ghandle.openGroup("dimnames");
        if (version >= 1100000) {
            lhandle.removeAttr("length");
        } else {
            lhandle.removeAttr("delayed_length");
        }
    }
    expect_error(path, "hello", "expected an attribute at");

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
        lhandle.createGroup("0");
    }
    expect_error(path, "hello", "expected a dataset at '0'");

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = dimnames_opener(fhandle, "hello", { 10, 20 }, "INTEGER", version);
        auto lhandle = ghandle.openGroup("dimnames");
        add_numeric_vector<int>(lhandle, "0", {1}, H5::PredType::NATIVE_INT32);
    }
    expect_error(path, "hello", "should be a 1-dimensional string dataset");

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
