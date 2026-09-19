#include <gtest/gtest.h>

#include <string>
#include <vector>
#include <cstddef>

#include "H5Cpp.h"
#include "ritsuko/ritsuko.hpp"
#include "chihaya/subset.hpp"

#include "utils.h"

static H5::Group subset_opener(
    H5::Group& handle,
    const std::string& name,
    const std::vector<std::size_t>& dimensions,
    const ritsuko::Version& version,
    const std::string& type
) {
    auto ghandle = operation_opener(handle, name, "subset");
    add_version_string(ghandle, version);
    mock_array_opener(ghandle, "seed", dimensions, version, type);
    return ghandle;
}

/********************************/

class SubsetPassTest : public ::testing::TestWithParam<std::tuple<ritsuko::Version, bool> > {};

TEST_P(SubsetPassTest, NoOp) {
    auto path = define_test_path("subset");
    auto params = GetParam();
    auto version = std::get<0>(params);
    auto deets = std::get<1>(params);

    std::vector<std::size_t> dims{ 24, 41 };
    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = subset_opener(fhandle, "hello", dims, version, "INTEGER");
        list_opener(ghandle, "index", 2, version);
    }

    auto output = test_validate(path, "hello", deets); 
    EXPECT_EQ(output.type, chihaya::INTEGER);
    EXPECT_EQ(output.dimensions, dims);
}

TEST_P(SubsetPassTest, AllSubsets) {
    auto path = define_test_path("subset");
    auto params = GetParam();
    auto version = std::get<0>(params);
    auto deets = std::get<1>(params);

    std::vector<int> first { 1, 3, 0, 2, 9, 12 };
    std::vector<int> second { 2, 2, 15, 7, 9, 9, 12 };
    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = subset_opener(fhandle, "hello", { 13, 19 }, version, "STRING");
        auto lhandle = list_opener(ghandle, "index", 2, version);

        if (version.lt(1, 1, 0)) {
            add_numeric_vector(lhandle, "0", first, H5::PredType::NATIVE_INT);
            add_numeric_vector(lhandle, "1", second, H5::PredType::NATIVE_INT);
        } else {
            add_numeric_vector(lhandle, "0", first, H5::PredType::NATIVE_UINT32);
            add_numeric_vector(lhandle, "1", second, H5::PredType::NATIVE_UINT8);
        }
    }

    auto output = test_validate(path, "hello", deets);
    EXPECT_EQ(output.type, chihaya::STRING);
    EXPECT_EQ(output.dimensions.size(), 2);
    EXPECT_EQ(output.dimensions[0], first.size());
    EXPECT_EQ(output.dimensions[1], second.size());
}

TEST_P(SubsetPassTest, OneSubset) {
    auto path = define_test_path("subset");
    auto params = GetParam();
    auto version = std::get<0>(params);
    auto deets = std::get<1>(params);

    std::vector<int> second{ 2, 2, 5, 7, 9, 9, 12 };
    std::vector<std::size_t> dims{ 2, 15 };
    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = subset_opener(fhandle, "hello", dims, version, "BOOLEAN");
        auto lhandle = list_opener(ghandle, "index", 2, version);

        if (version.lt(1, 1, 0)) {
            add_numeric_vector(lhandle, "1", second, H5::PredType::NATIVE_INT);
        } else {
            add_numeric_vector(lhandle, "1", second, H5::PredType::NATIVE_UINT16);
        }
    }

    auto output = test_validate(path, "hello", deets);
    EXPECT_EQ(output.type, chihaya::BOOLEAN);
    EXPECT_EQ(output.dimensions.size(), 2);
    EXPECT_EQ(output.dimensions[0], dims[0]);
    EXPECT_EQ(output.dimensions[1], second.size());
}

TEST_P(SubsetPassTest, TwoSubset) {
    auto path = define_test_path("subset");
    auto params = GetParam();
    auto version = std::get<0>(params);
    auto deets = std::get<1>(params);

    // Getting some coverage of a 3-dimensional array.
    std::vector<std::size_t> dims{ 5, 1, 10 };
    std::vector<int> first{ 0, 1, 0, 2, 0, 3 };
    std::vector<int> last{ 7, 3, 1, 8, 2, 3 };

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = subset_opener(fhandle, "hello", dims, version, "FLOAT");
        auto lhandle = list_opener(ghandle, "index", 3, version);

        if (version.lt(1, 1, 0)) {
            add_numeric_vector(lhandle, "0", first, H5::PredType::NATIVE_INT);
            add_numeric_vector(lhandle, "2", last, H5::PredType::NATIVE_INT);
        } else {
            add_numeric_vector(lhandle, "0", first, H5::PredType::NATIVE_UINT8);
            add_numeric_vector(lhandle, "2", last, H5::PredType::NATIVE_UINT8);
        }
    }

    auto output = test_validate(path, "hello", deets);
    EXPECT_EQ(output.type, chihaya::FLOAT);
    EXPECT_EQ(output.dimensions.size(), 3);
    EXPECT_EQ(output.dimensions[0], first.size());
    EXPECT_EQ(output.dimensions[1], dims[1]);
    EXPECT_EQ(output.dimensions[2], last.size());
}

INSTANTIATE_TEST_SUITE_P(
    Subset,
    SubsetPassTest,
    ::testing::Combine(
        spawn_all_versions(),
        ::testing::Values(false, true)
    )
);

/********************************/

class SubsetErrorTest : public ::testing::TestWithParam<ritsuko::Version> {};

TEST_P(SubsetErrorTest, Index) {
    auto path = define_test_path("subset");
    auto version = GetParam();

    // Check that the subset list validation is actually called.
    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = subset_opener(fhandle, "hello", { 13, 19 }, version, "BOOLEAN");
        auto lhandle = list_opener(ghandle, "index", 2, version);
        add_numeric_vector<int>(lhandle, "2", { 1, 3, 0, 2, 9 }, H5::PredType::NATIVE_UINT16);
    }
    expect_error(path, "hello", "out of range");

    {
        H5::H5File fhandle(path, H5F_ACC_RDWR);
        auto ghandle = fhandle.openGroup("hello");
        auto lhandle = ghandle.openGroup("index");
        lhandle.unlink("2"); // removing the above.
        add_numeric_vector<int>(lhandle, "1", { 1, 3, 0, 2, 1009 }, H5::PredType::NATIVE_UINT32);
    }
    expect_error(path, "hello", "indices out of range");
}

INSTANTIATE_TEST_SUITE_P(
    Subset,
    SubsetErrorTest,
    spawn_all_versions()
);
