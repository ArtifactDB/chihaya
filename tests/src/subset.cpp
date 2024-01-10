#include <gtest/gtest.h>
#include "chihaya/chihaya.hpp"
#include "utils.h"

class SubsetTest : public ::testing::TestWithParam<int> {
public:
    SubsetTest() : path("Test_subset.h5") {}

protected:
    std::string path;

    static H5::Group subset_opener(H5::Group& handle, const std::string& name, const std::vector<int>& dimensions, int version, std::string type) {
        auto ghandle = operation_opener(handle, name, "subset");
        add_version_string(ghandle, version);
        mock_array_opener(ghandle, "seed", dimensions, version, type);
        return ghandle;
    }
};

TEST_P(SubsetTest, NoOp) {
    auto version = GetParam();

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = subset_opener(fhandle, "hello", { 13, 19 }, version, "INTEGER");
        list_opener(ghandle, "index", 2, version);
    }

    auto output = test_validate(path, "hello"); 
    EXPECT_EQ(output.type, chihaya::INTEGER);
    const auto& dims = output.dimensions;
    EXPECT_EQ(dims[0], 13);
    EXPECT_EQ(dims[1], 19);
}

TEST_P(SubsetTest, AllSubsets) {
    auto version = GetParam();
    std::vector<int> first { 1, 3, 0, 2, 9, 12 };
    std::vector<int> second { 2, 2, 15, 7, 9, 9, 12 };

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = subset_opener(fhandle, "hello", { 13, 19 }, version, "STRING");
        auto lhandle = list_opener(ghandle, "index", 2, version);

        if (version < 1100000) {
            add_numeric_vector(lhandle, "0", first, H5::PredType::NATIVE_INT);
            add_numeric_vector(lhandle, "1", second, H5::PredType::NATIVE_INT);
        } else {
            add_numeric_vector(lhandle, "0", first, H5::PredType::NATIVE_UINT32);
            add_numeric_vector(lhandle, "1", second, H5::PredType::NATIVE_UINT8);
        }
    }

    auto output = test_validate(path, "hello"); 
    EXPECT_EQ(output.type, chihaya::STRING);
    const auto& dims = output.dimensions;
    EXPECT_EQ(dims[0], first.size());
    EXPECT_EQ(dims[1], second.size());
}

TEST_P(SubsetTest, OneSubset) {
    auto version = GetParam();
    std::vector<int> second{ 2, 2, 5, 7, 9, 9, 12 };

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = subset_opener(fhandle, "hello", { 13, 19 }, version, "BOOLEAN");
        auto lhandle = list_opener(ghandle, "index", 2, version);

        if (version < 1100000) {
            add_numeric_vector(lhandle, "1", second, H5::PredType::NATIVE_INT);
        } else {
            add_numeric_vector(lhandle, "1", second, H5::PredType::NATIVE_UINT16);
        }
    }

    auto output = test_validate(path, "hello"); 
    EXPECT_EQ(output.type, chihaya::BOOLEAN);
    const auto& dims = output.dimensions;
    EXPECT_EQ(dims[0], 13);
    EXPECT_EQ(dims[1], 7);
}

TEST_P(SubsetTest, Errors) {
    auto version = GetParam();

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = subset_opener(fhandle, "hello", { 13, 19 }, version, "BOOLEAN");
        ghandle.unlink("seed");
    }
    expect_error(path, "hello", "expected a group at 'seed'");

    {
        H5::H5File fhandle(path, H5F_ACC_RDWR);
        auto ghandle = fhandle.openGroup("hello");
        mock_array_opener(ghandle, "seed", { 13, 19 }, version, "INTEGER"); 
    }
    expect_error(path, "hello", "expected a group at 'index'");

    {
        H5::H5File fhandle(path, H5F_ACC_RDWR);
        auto ghandle = fhandle.openGroup("hello");
        auto lhandle = list_opener(ghandle, "index", 2, version);
        add_numeric_vector<int>(lhandle, "2", { 1, 3, 0, 2, 9 }, H5::PredType::NATIVE_UINT16);
    }
    expect_error(path, "hello", "out of range");

    {
        H5::H5File fhandle(path, H5F_ACC_RDWR);
        auto ghandle = fhandle.openGroup("hello");
        auto lhandle = ghandle.openGroup("index");
        lhandle.unlink("2"); // removing the above.
        lhandle.createGroup("0");
    }
    expect_error(path, "hello", "expected a dataset at '0'");

    {
        H5::H5File fhandle(path, H5F_ACC_RDWR);
        auto ghandle = fhandle.openGroup("hello");
        auto lhandle = ghandle.openGroup("index");
        lhandle.unlink("0"); // removing the above.
        add_numeric_vector<int>(lhandle, "1", { 1, 3, 0, 2, 9 }, H5::PredType::NATIVE_DOUBLE);
    }
    if (version < 1100000) {
        expect_error(path, "hello", "expected an integer dataset");
    } else {
        expect_error(path, "hello", "64-bit unsigned integer");
    }

    {
        H5::H5File fhandle(path, H5F_ACC_RDWR);
        auto ghandle = fhandle.openGroup("hello");
        auto lhandle = ghandle.openGroup("index");
        lhandle.unlink("1"); // removing the above.
        add_numeric_vector<int>(lhandle, "1", { 1, 3, 0, -2, 9 }, H5::PredType::NATIVE_INT);
    }
    if (version < 1100000) {
        expect_error(path, "hello", "non-negative");
    } else {
        expect_error(path, "hello", "64-bit unsigned integer");
    }

    {
        H5::H5File fhandle(path, H5F_ACC_RDWR);
        auto ghandle = fhandle.openGroup("hello");
        auto lhandle = ghandle.openGroup("index");
        lhandle.unlink("1"); // removing the above.
        add_numeric_vector<int>(lhandle, "1", { 1, 3, 0, 2, 1009 }, H5::PredType::NATIVE_UINT32);
    }
    expect_error(path, "hello", "indices out of range");
}

INSTANTIATE_TEST_SUITE_P(
    Subset,
    SubsetTest,
    ::testing::Values(0, 1000000, 1100000)
);

