#include <gtest/gtest.h>
#include "chihaya/chihaya.hpp"
#include "utils.h"

class TransposeTest : public ::testing::TestWithParam<int> {
public:
    TransposeTest() : path("Test_transpose.h5") {}

protected:
    std::string path;

    static H5::Group transpose_opener(H5::Group& handle, const std::string& name, const std::vector<int>& dimensions, int version, std::string type) {
        auto ghandle = operation_opener(handle, name, "transpose");
        add_version_string(ghandle, version);
        mock_array_opener(ghandle, "seed", dimensions, version, type);
        return ghandle;
    }
};

TEST_P(TransposeTest, NoOp) {
    auto version = GetParam();

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = transpose_opener(fhandle, "hello", { 13, 19 }, version, "INTEGER"); 
        if (version < 1100000) {
            add_numeric_vector<int>(ghandle, "permutation", { 0, 1 }, H5::PredType::NATIVE_INT);
        } else {
            add_numeric_vector<int>(ghandle, "permutation", { 0, 1 }, H5::PredType::NATIVE_UINT32);
        }
    }

    auto output = chihaya::validate(path, "hello"); 
    EXPECT_EQ(output.type, chihaya::INTEGER);
    const auto& dims = output.dimensions;
    EXPECT_EQ(dims[0], 13);
    EXPECT_EQ(dims[1], 19);
}

TEST_P(TransposeTest, Simple) {
    auto version = GetParam();

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = transpose_opener(fhandle, "hello", { 13, 19 }, version, "STRING"); 
        if (version < 1100000) {
            add_numeric_vector<int>(ghandle, "permutation", { 1, 0 }, H5::PredType::NATIVE_INT);
        } else {
            add_numeric_vector<int>(ghandle, "permutation", { 1, 0 }, H5::PredType::NATIVE_UINT32);
        }
    }

    auto output = chihaya::validate(path, "hello"); 
    EXPECT_EQ(output.type, chihaya::STRING);
    const auto& dims = output.dimensions;
    EXPECT_EQ(dims[0], 19);
    EXPECT_EQ(dims[1], 13);
}

TEST_P(TransposeTest, Complex) {
    auto version = GetParam();

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = transpose_opener(fhandle, "hello", { 13, 19, 5 }, version, "BOOLEAN"); 
        if (version < 1100000) {
            add_numeric_vector<int>(ghandle, "permutation", { 1, 2, 0 }, H5::PredType::NATIVE_INT);
        } else {
            add_numeric_vector<int>(ghandle, "permutation", { 1, 2, 0 }, H5::PredType::NATIVE_UINT32);
        }
    }

    auto output = chihaya::validate(path, "hello"); 
    EXPECT_EQ(output.type, chihaya::BOOLEAN);
    const auto& dims = output.dimensions;
    EXPECT_EQ(dims[0], 19);
    EXPECT_EQ(dims[1], 5);
    EXPECT_EQ(dims[2], 13);
}

TEST_P(TransposeTest, Errors) {
    auto version = GetParam();

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        transpose_opener(fhandle, "hello", { 13, 19 }, version, "INTEGER"); 
    }
    expect_error([&]() -> void { chihaya::validate(path, "hello"); }, "expected a dataset at 'permutation'");

    {
        H5::H5File fhandle(path, H5F_ACC_RDWR);
        auto ghandle = fhandle.openGroup("hello");
        ghandle.unlink("seed");
    }
    expect_error([&]() -> void { chihaya::validate(path, "hello"); }, "expected a group at 'seed'");
}

TEST_P(TransposeTest, PermutationErrors) {
    auto version = GetParam();

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = transpose_opener(fhandle, "hello", { 13, 19 }, version, "INTEGER"); 
        add_numeric_vector<int>(ghandle, "permutation", { 1, 2, 0 }, H5::PredType::NATIVE_DOUBLE);
    }
    if (version < 1100000) {
        expect_error([&]() -> void { chihaya::validate(path, "hello"); }, "'permutation' should be integer");
    } else {
        expect_error([&]() -> void { chihaya::validate(path, "hello"); }, "64-bit unsigned integer");
    }

    {
        H5::H5File fhandle(path, H5F_ACC_RDWR);
        auto ghandle = fhandle.openGroup("hello");
        ghandle.unlink("permutation");
        add_numeric_vector<int>(ghandle, "permutation", { 1, 2, 0 }, H5::PredType::NATIVE_UINT32);
    }
    expect_error([&]() -> void { chihaya::validate(path, "hello"); }, "length of 'permutation'");

    {
        H5::H5File fhandle(path, H5F_ACC_RDWR);
        auto ghandle = fhandle.openGroup("hello");
        ghandle.unlink("permutation");
        add_numeric_vector<int>(ghandle, "permutation", { 1, 5 }, H5::PredType::NATIVE_UINT8);
    }
    expect_error([&]() -> void { chihaya::validate(path, "hello"); }, "out-of-bounds");

    {
        H5::H5File fhandle(path, H5F_ACC_RDWR);
        auto ghandle = fhandle.openGroup("hello");
        ghandle.unlink("permutation");
        add_numeric_vector<int>(ghandle, "permutation", { -1, 0 }, H5::PredType::NATIVE_INT);
    }
    if (version < 1100000) {
        expect_error([&]() -> void { chihaya::validate(path, "hello"); }, "non-negative");
    } else {
        expect_error([&]() -> void { chihaya::validate(path, "hello"); }, "64-bit unsigned integer");
    }

    {
        H5::H5File fhandle(path, H5F_ACC_RDWR);
        auto ghandle = fhandle.openGroup("hello");
        ghandle.unlink("permutation");
        add_numeric_vector<int>(ghandle, "permutation", { 0, 0 }, H5::PredType::NATIVE_UINT8);
    }
    expect_error([&]() -> void { chihaya::validate(path, "hello"); }, "unique");
}

INSTANTIATE_TEST_SUITE_P(
    Transpose,
    TransposeTest,
    ::testing::Values(0, 1000000, 1100000)
);
