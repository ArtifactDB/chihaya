#include <gtest/gtest.h>

#include <string>
#include <vector>
#include <cstddef>

#include "H5Cpp.h"
#include "ritsuko/ritsuko.hpp"
#include "chihaya/transpose.hpp"

#include "utils.h"

static H5::Group transpose_opener(
    H5::Group& handle,
    const std::string& name,
    const std::vector<std::size_t>& dimensions,
    const ritsuko::Version& version,
    const std::string& type
) {
    auto ghandle = operation_opener(handle, name, "transpose");
    add_version_string(ghandle, version);
    mock_array_opener(ghandle, "seed", dimensions, version, type);
    return ghandle;
}

/********************************/

class TransposePassTest : public ::testing::TestWithParam<std::tuple<ritsuko::Version, bool> > {};

TEST_P(TransposePassTest, NoOp) {
    auto path = define_test_path("transpose");
    auto params = GetParam();
    auto version = std::get<0>(params);
    auto deets = std::get<1>(params);

    std::vector<std::size_t> dims{ 28, 13 };
    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = transpose_opener(fhandle, "hello", dims, version, "INTEGER"); 
        if (version.lt(1, 1, 0)) {
            add_numeric_vector<int>(ghandle, "permutation", { 0, 1 }, H5::PredType::NATIVE_INT);
        } else {
            add_numeric_vector<int>(ghandle, "permutation", { 0, 1 }, H5::PredType::NATIVE_UINT32);
        }
    }
    {
        auto output = test_validate(path, "hello", deets); 
        EXPECT_EQ(output.type, chihaya::INTEGER);
        EXPECT_EQ(output.dimensions, dims);
    }

    // Using an unsigned integer datatype in HDF5 to get some coverage of the legacy unsigned case.
    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = transpose_opener(fhandle, "hello", dims, version, "INTEGER"); 
        add_numeric_vector<int>(ghandle, "permutation", { 0, 1 }, H5::PredType::NATIVE_UINT8);
    }
    {
        auto output = test_validate(path, "hello", deets); 
        EXPECT_EQ(output.type, chihaya::INTEGER);
        EXPECT_EQ(output.dimensions, dims);
    }
}

TEST_P(TransposePassTest, Simple) {
    auto path = define_test_path("transpose");
    auto params = GetParam();
    auto version = std::get<0>(params);
    auto deets = std::get<1>(params);

    std::vector<std::size_t> dims{ 31, 17 };
    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = transpose_opener(fhandle, "hello", dims, version, "STRING"); 
        if (version.lt(1, 1, 0)) {
            add_numeric_vector<int>(ghandle, "permutation", { 1, 0 }, H5::PredType::NATIVE_INT);
        } else {
            add_numeric_vector<int>(ghandle, "permutation", { 1, 0 }, H5::PredType::NATIVE_UINT32);
        }
    }

    auto output = test_validate(path, "hello", deets);
    EXPECT_EQ(output.type, chihaya::STRING);
    EXPECT_EQ(output.dimensions.size(), 2);
    EXPECT_EQ(output.dimensions[0], dims[1]);
    EXPECT_EQ(output.dimensions[1], dims[0]);
}

TEST_P(TransposePassTest, Complicated) {
    auto path = define_test_path("transpose");
    auto params = GetParam();
    auto version = std::get<0>(params);
    auto deets = std::get<1>(params);

    std::vector<std::size_t> dims{ 13, 29, 5 };
    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = transpose_opener(fhandle, "hello", dims, version, "BOOLEAN"); 
        if (version.lt(1, 1, 0)) {
            add_numeric_vector<int>(ghandle, "permutation", { 1, 2, 0 }, H5::PredType::NATIVE_INT);
        } else {
            add_numeric_vector<int>(ghandle, "permutation", { 1, 2, 0 }, H5::PredType::NATIVE_UINT32);
        }
    }

    auto output = test_validate(path, "hello", deets);
    EXPECT_EQ(output.type, chihaya::BOOLEAN);
    EXPECT_EQ(output.dimensions.size(), 3);
    EXPECT_EQ(output.dimensions[0], dims[1]);
    EXPECT_EQ(output.dimensions[1], dims[2]);
    EXPECT_EQ(output.dimensions[2], dims[0]);
}

INSTANTIATE_TEST_SUITE_P(
    Transpose,
    TransposePassTest,
    ::testing::Combine(
        spawn_all_versions(),
        ::testing::Values(false, true)
    )
);

/********************************/

class TransposeErrorTest : public ::testing::TestWithParam<ritsuko::Version> {};

TEST_P(TransposeErrorTest, Permutation) {
    auto path = define_test_path("transpose");
    auto version = GetParam();

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = transpose_opener(fhandle, "hello", { 13, 19 }, version, "INTEGER"); 
        hsize_t dims[2] = { 13, 15 };
        ghandle.createDataSet("permutation", H5::PredType::NATIVE_UINT8, H5::DataSpace(2, dims));
    }
    expect_error(path, "hello", "1-dimensional");

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = transpose_opener(fhandle, "hello", { 13, 19 }, version, "INTEGER"); 
        add_numeric_vector<int>(ghandle, "permutation", { 1, 0 }, H5::PredType::NATIVE_DOUBLE);
    }
    if (version.lt(1, 1, 0)) {
        expect_error(path, "hello", "expected an integer type");
    } else {
        expect_error(path, "hello", "64-bit unsigned integer");
    }

    {
        H5::H5File fhandle(path, H5F_ACC_RDWR);
        auto ghandle = fhandle.openGroup("hello");
        ghandle.unlink("permutation");
        add_numeric_vector<int>(ghandle, "permutation", { 1, 2, 0 }, H5::PredType::NATIVE_UINT32);
    }
    expect_error(path, "hello", "length of 'permutation'");

    {
        H5::H5File fhandle(path, H5F_ACC_RDWR);
        auto ghandle = fhandle.openGroup("hello");
        ghandle.unlink("permutation");
        add_numeric_vector<int>(ghandle, "permutation", { 1, 5 }, H5::PredType::NATIVE_UINT8);
    }
    expect_error(path, "hello", "out-of-bounds");

    {
        H5::H5File fhandle(path, H5F_ACC_RDWR);
        auto ghandle = fhandle.openGroup("hello");
        ghandle.unlink("permutation");
        add_numeric_vector<int>(ghandle, "permutation", { -1, 0 }, H5::PredType::NATIVE_INT);
    }
    if (version.lt(1, 1, 0)) {
        expect_error(path, "hello", "non-negative");
    } else {
        expect_error(path, "hello", "64-bit unsigned integer");
    }

    {
        H5::H5File fhandle(path, H5F_ACC_RDWR);
        auto ghandle = fhandle.openGroup("hello");
        ghandle.unlink("permutation");
        add_numeric_vector<int>(ghandle, "permutation", { 0, 0 }, H5::PredType::NATIVE_UINT8);
    }
    expect_error(path, "hello", "unique");
}

INSTANTIATE_TEST_SUITE_P(
    Transpose,
    TransposeErrorTest,
    spawn_all_versions()
);
