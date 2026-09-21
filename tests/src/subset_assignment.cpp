#include <gtest/gtest.h>

#include <vector>
#include <cstddef>
#include <string>

#include "H5Cpp.h"
#include "ritsuko/ritsuko.hpp"
#include "chihaya/subset_assignment.hpp"

#include "utils.h"

static H5::Group subset_assignment_opener(
    H5::Group& handle,
    const std::string& name,
    const std::vector<std::size_t>& dimensions,
    const ritsuko::Version& version,
    const std::string& type
) {
    auto ghandle = operation_opener(handle, name, "subset assignment");
    add_version_string(ghandle, version);
    mock_array_opener(ghandle, "seed", dimensions, version, type);
    return ghandle;
}

/********************************/

class SubsetAssignmentTest : public ::testing::TestWithParam<std::tuple<ritsuko::Version, bool> > {};

TEST_P(SubsetAssignmentTest, FullSubstitution) {
    auto path = define_test_path("subset_assignment");
    auto params = GetParam();
    auto version = std::get<0>(params);
    auto deets = std::get<1>(params);

    // Without any subset indices, we use the full extent of each dimension, so this is a full replacement of 'seed' by 'value'.
    // A bit silly to have a delayed operation for this but it's the natural edge case.
    std::vector<std::size_t> dims{ 14, 21 };
    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = subset_assignment_opener(fhandle, "hello", dims, version, "INTEGER");
        mock_array_opener(ghandle, "value", dims, version, "INTEGER");
        auto lhandle = list_opener(ghandle, "index", 2, version);
    }

    auto output = test_validate(path, "hello", deets); 
    EXPECT_EQ(output.type, chihaya::INTEGER);
    EXPECT_EQ(output.dimensions, dims);
}

TEST_P(SubsetAssignmentTest, AllSubsets) {
    auto path = define_test_path("subset_assignment");
    auto params = GetParam();
    auto version = std::get<0>(params);
    auto deets = std::get<1>(params);

    std::vector<std::size_t> dims{ 13, 19 };
    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = subset_assignment_opener(fhandle, "hello", dims, version, "STRING");

        auto lhandle = list_opener(ghandle, "index", 2, version);
        std::vector<int> first{ 1, 3, 0, 2, 9, 12 };
        std::vector<int> second{ 2, 2, 5, 17, 9, 9, 12 };
        if (version.lt(1, 1, 0)) {
            add_numeric_vector(lhandle, "0", first, H5::PredType::NATIVE_INT);
            add_numeric_vector(lhandle, "1", second, H5::PredType::NATIVE_INT);
        } else {
            add_numeric_vector(lhandle, "0", first, H5::PredType::NATIVE_UINT8);
            add_numeric_vector(lhandle, "1", second, H5::PredType::NATIVE_UINT16);
        }

        const int first_size = first.size(), second_size = second.size();
        mock_array_opener<int>(ghandle, "value", { first_size, second_size }, version, "STRING"); 
    }

    auto output = test_validate(path, "hello", deets); 
    EXPECT_EQ(output.type, chihaya::STRING);
    EXPECT_EQ(output.dimensions, dims);
}

TEST_P(SubsetAssignmentTest, OneSubset) {
    auto path = define_test_path("subset_assignment");
    auto params = GetParam();
    auto version = std::get<0>(params);
    auto deets = std::get<1>(params);

    std::vector<std::size_t> dims{ 20, 12 };
    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = subset_assignment_opener(fhandle, "hello", dims, version, "BOOLEAN");

        auto lhandle = list_opener(ghandle, "index", 2, version);
        std::vector<int> second{ 2, 2, 5, 11, 9, 9, 0 };
        if (version.lt(1, 1, 0)) {
            add_numeric_vector(lhandle, "1", second, H5::PredType::NATIVE_INT);
        } else {
            add_numeric_vector(lhandle, "1", second, H5::PredType::NATIVE_UINT16);
        }

        auto valdim = dims;
        valdim[1] = second.size();
        mock_array_opener<std::size_t>(ghandle, "value", valdim, version, "FLOAT"); 
    }

    auto output = test_validate(path, "hello", deets); 
    EXPECT_EQ(output.type, chihaya::FLOAT);
    EXPECT_EQ(output.dimensions, dims);
}

TEST_P(SubsetAssignmentTest, TwoSubsets) {
    auto path = define_test_path("subset_assignment");
    auto params = GetParam();
    auto version = std::get<0>(params);
    auto deets = std::get<1>(params);

    // Trying a higher-dimensional seed, for some variety.
    std::vector<std::size_t> dims{ 10, 12, 20 };
    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = subset_assignment_opener(fhandle, "hello", dims, version, "FLOAT");

        auto lhandle = list_opener(ghandle, "index", 3, version);
        std::vector<int> first{ 1, 9, 8, 4, 5, 2, 9, 9 };
        std::vector<int> last{ 19, 16, 17, 10, 5, 2, 7, 8 };
        if (version.lt(1, 1, 0)) {
            add_numeric_vector(lhandle, "0", first, H5::PredType::NATIVE_INT);
            add_numeric_vector(lhandle, "2", last, H5::PredType::NATIVE_UINT);
        } else {
            add_numeric_vector(lhandle, "0", first, H5::PredType::NATIVE_UINT16);
            add_numeric_vector(lhandle, "2", last, H5::PredType::NATIVE_UINT8);
        }

        auto valdim = dims;
        valdim[0] = first.size();
        valdim[2] = last.size();
        mock_array_opener<std::size_t>(ghandle, "value", valdim, version, "INTEGER"); 
    }

    auto output = test_validate(path, "hello", deets); 
    EXPECT_EQ(output.type, chihaya::FLOAT);
    EXPECT_EQ(output.dimensions, dims);
}

INSTANTIATE_TEST_SUITE_P(
    SubsetAssignment,
    SubsetAssignmentTest,
    ::testing::Combine(
        spawn_all_versions(),
        ::testing::Values(false, true)
    )
);

/********************************/

class SubsetAssignmentErrorTest : public ::testing::TestWithParam<ritsuko::Version> {};

TEST_P(SubsetAssignmentErrorTest, Seed) {
    auto path = define_test_path("subset_assignment");
    auto version = GetParam();

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = subset_assignment_opener(fhandle, "hello", { 13, 19 }, version, "BOOLEAN");
        mock_array_opener<int>(ghandle, "value", { 5, 4 }, version, "STRING");
    }
    expect_error(path, "hello", "both or neither");

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = subset_assignment_opener(fhandle, "hello", { 13, 19 }, version, "STRING");
        mock_array_opener<int>(ghandle, "value", { 5, 4 }, version, "FLOAT");
    }
    expect_error(path, "hello", "both or neither");

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = subset_assignment_opener(fhandle, "hello", { 13, 19, 4 }, version, "FLOAT");
        mock_array_opener<int>(ghandle, "value", { 5, 4 }, version, "FLOAT");
    }
    expect_error(path, "hello", "same dimensionality");
}

TEST_P(SubsetAssignmentErrorTest, Index) {
    auto path = define_test_path("subset_assignment");
    auto version = GetParam();

    // Check that the subset index list is actually validated.
    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = subset_assignment_opener(fhandle, "hello", { 13, 19 }, version, "BOOLEAN");
        mock_array_opener<int>(ghandle, "value", { 5, 19 }, version, "INTEGER"); 
        auto lhandle = list_opener(ghandle, "index", 2, version);
        add_numeric_vector<int>(lhandle, "2", { 1, 3, 0, 2, 9 }, H5::PredType::NATIVE_INT);
    }
    expect_error(path, "hello", "out of range");

    {
        H5::H5File fhandle(path, H5F_ACC_RDWR);
        auto ghandle = fhandle.openGroup("hello");
        auto lhandle = ghandle.openGroup("index");
        lhandle.unlink("2"); // removing the above.
        add_numeric_vector<int>(lhandle, "0", { 1, 3, 0, 2, 1, 9, 5 }, H5::PredType::NATIVE_UINT16);
    }
    expect_error(path, "hello", "dimension extents are not consistent");
}

INSTANTIATE_TEST_SUITE_P(
    SubsetAssignment,
    SubsetAssignmentErrorTest,
    spawn_all_versions()
);
