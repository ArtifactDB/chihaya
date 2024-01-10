#include <gtest/gtest.h>
#include "chihaya/chihaya.hpp"
#include "utils.h"

class SubsetAssignmentTest : public ::testing::TestWithParam<int> {
public:
    SubsetAssignmentTest() : path("Test_subset_assignment.h5") {}

protected:
    std::string path;

    static H5::Group subset_assignment_opener(H5::Group& handle, const std::string& name, const std::vector<int>& dimensions, int version, std::string type) {
        auto ghandle = operation_opener(handle, name, "subset assignment");
        add_version_string(ghandle, version);
        mock_array_opener(ghandle, "seed", dimensions, version, type);
        return ghandle;
    }
};

TEST_P(SubsetAssignmentTest, NoOp) {
    auto version = GetParam();

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = subset_assignment_opener(fhandle, "hello", { 13, 19 }, version, "INTEGER");
        mock_array_opener(ghandle, "value", { 13, 19 }, version, "INTEGER"); 
        auto lhandle = list_opener(ghandle, "index", 2, version);
    }

    auto output = chihaya::validate(path, "hello"); 
    EXPECT_EQ(output.type, chihaya::INTEGER);
    const auto& dims = output.dimensions;
    EXPECT_EQ(dims[0], 13);
    EXPECT_EQ(dims[1], 19);
}

TEST_P(SubsetAssignmentTest, AllSubsets) {
    auto version = GetParam();

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = subset_assignment_opener(fhandle, "hello", { 13, 19 }, version, "STRING");

        auto lhandle = list_opener(ghandle, "index", 2, version);
        std::vector<int> first{ 1, 3, 0, 2, 9, 12 };
        std::vector<int> second{ 2, 2, 5, 17, 9, 9, 12 };
        if (version < 1100000) {
            add_numeric_vector(lhandle, "0", first, H5::PredType::NATIVE_INT);
            add_numeric_vector(lhandle, "1", second, H5::PredType::NATIVE_INT);
        } else {
            add_numeric_vector(lhandle, "0", first, H5::PredType::NATIVE_UINT8);
            add_numeric_vector(lhandle, "1", second, H5::PredType::NATIVE_UINT16);
        }

        int first_size = first.size(), second_size = second.size();
        mock_array_opener(ghandle, "value", { first_size, second_size }, version, "STRING"); 
    }

    auto output = chihaya::validate(path, "hello"); 
    EXPECT_EQ(output.type, chihaya::STRING);
    const auto& dims = output.dimensions;
    EXPECT_EQ(dims[0], 13);
    EXPECT_EQ(dims[1], 19);
}

TEST_P(SubsetAssignmentTest, OneSubset) {
    auto version = GetParam();

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = subset_assignment_opener(fhandle, "hello", { 13, 19 }, version, "BOOLEAN");

        auto lhandle = list_opener(ghandle, "index", 2, version);
        std::vector<int> second{ 2, 2, 5, 17, 9, 9, 12 };
        if (version < 1100000) {
            add_numeric_vector(lhandle, "1", second, H5::PredType::NATIVE_INT);
        } else {
            add_numeric_vector(lhandle, "1", second, H5::PredType::NATIVE_UINT16);
        }

        int second_size = second.size();
        mock_array_opener(ghandle, "value", { 13, second_size }, version, "FLOAT"); 
    }

    auto output = chihaya::validate(path, "hello"); 
    EXPECT_EQ(output.type, chihaya::FLOAT);
    const auto& dims = output.dimensions;
    EXPECT_EQ(dims[0], 13);
    EXPECT_EQ(dims[1], 19);
}

TEST_P(SubsetAssignmentTest, Errors) {
    auto version = GetParam();

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = subset_assignment_opener(fhandle, "hello", { 13, 19 }, version, "BOOLEAN");
        ghandle.unlink("seed");
    }
    expect_error([&]() -> void { chihaya::validate(path, "hello"); }, "expected a group at 'seed'");

    {
        H5::H5File fhandle(path, H5F_ACC_RDWR);
        auto ghandle = fhandle.openGroup("hello");
        mock_array_opener(ghandle, "seed", { 13, 19 }, version, "INTEGER");
    }
    expect_error([&]() -> void { chihaya::validate(path, "hello"); }, "expected a group at 'value'");

    {
        H5::H5File fhandle(path, H5F_ACC_RDWR);
        auto ghandle = fhandle.openGroup("hello");
        mock_array_opener(ghandle, "value", { 5, 4 }, version, "STRING");
    }
    expect_error([&]() -> void { chihaya::validate(path, "hello"); }, "both or neither");
}

TEST_P(SubsetAssignmentTest, IndexErrors) {
    auto version = GetParam();

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = subset_assignment_opener(fhandle, "hello", { 13, 19 }, version, "BOOLEAN");
        mock_array_opener(ghandle, "value", { 5, 19 }, version, "INTEGER"); 
    }
    expect_error([&]() -> void { chihaya::validate(path, "hello"); }, "expected a group at 'index'");

    {
        H5::H5File fhandle(path, H5F_ACC_RDWR);
        auto ghandle = fhandle.openGroup("hello");
        auto lhandle = list_opener(ghandle, "index", 2, version);
        add_numeric_vector<int>(lhandle, "2", { 1, 3, 0, 2, 9 }, H5::PredType::NATIVE_INT);
    }
    expect_error([&]() -> void { chihaya::validate(path, "hello"); }, "out of range");

    {
        H5::H5File fhandle(path, H5F_ACC_RDWR);
        auto ghandle = fhandle.openGroup("hello");
        auto lhandle = ghandle.openGroup("index");
        lhandle.unlink("2"); // removing the above.
        lhandle.createGroup("0");
    }
    expect_error([&]() -> void { chihaya::validate(path, "hello"); }, "expected a dataset");

    {
        H5::H5File fhandle(path, H5F_ACC_RDWR);
        auto ghandle = fhandle.openGroup("hello");
        auto lhandle = ghandle.openGroup("index");
        lhandle.unlink("0"); // removing the above.
        add_numeric_vector<int>(lhandle, "0", { 1, 3, 0, 2, 9 }, H5::PredType::NATIVE_DOUBLE);
    }
    if (version < 1100000) {
        expect_error([&]() -> void { chihaya::validate(path, "hello"); }, "expected an integer dataset");
    } else {
        expect_error([&]() -> void { chihaya::validate(path, "hello"); }, "64-bit unsigned integer");
    }

    {
        H5::H5File fhandle(path, H5F_ACC_RDWR);
        auto ghandle = fhandle.openGroup("hello");
        auto lhandle = ghandle.openGroup("index");
        lhandle.unlink("0"); // removing the above.
        add_numeric_vector<int>(lhandle, "0", { 1, -3, 0, 2, 9 }, H5::PredType::NATIVE_INT);
    }
    if (version < 1100000) {
        expect_error([&]() -> void { chihaya::validate(path, "hello"); }, "should be non-negative");
    } else {
        expect_error([&]() -> void { chihaya::validate(path, "hello"); }, "64-bit unsigned integer");
    }

    {
        H5::H5File fhandle(path, H5F_ACC_RDWR);
        auto ghandle = fhandle.openGroup("hello");
        auto lhandle = ghandle.openGroup("index");
        lhandle.unlink("0"); // removing the above.
        add_numeric_vector<int>(lhandle, "0", { 1, 3, 0, 2, 1, 9, 5 }, H5::PredType::NATIVE_UINT16);
    }
    expect_error([&]() -> void { chihaya::validate(path, "hello"); }, "dimension extents are not consistent");

    {
        H5::H5File fhandle(path, H5F_ACC_RDWR);
        auto ghandle = fhandle.openGroup("hello");
        auto lhandle = ghandle.openGroup("index");
        lhandle.unlink("0"); // removing the above.
        add_numeric_vector<int>(lhandle, "0", { 1, 3, 0, 2, 200 }, H5::PredType::NATIVE_UINT16);
    }
    expect_error([&]() -> void { chihaya::validate(path, "hello"); }, "out of range");
}

INSTANTIATE_TEST_SUITE_P(
    SubsetAssignment,
    SubsetAssignmentTest,
    ::testing::Values(0, 1000000, 1100000)
);
