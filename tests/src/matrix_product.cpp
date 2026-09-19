#include <gtest/gtest.h>

#include <string>
#include <vector>
#include <cstddef>

#include "H5Cpp.h"
#include "ritsuko/ritsuko.hpp"
#include "chihaya/matrix_product.hpp"

#include "utils.h"

static H5::Group matrix_product_opener(H5::Group& handle, const std::string& name, const ritsuko::Version& version) {
    auto ghandle = operation_opener(handle, name, "matrix product");
    add_version_string(ghandle, version);
    return ghandle;
}

static void add_seed(H5::Group& parent, const std::vector<int>& dimensions, const ritsuko::Version& version, std::string type, bool left, bool transposed) {
    std::string thing = (left ? std::string("left") : std::string("right"));
    mock_array_opener(parent, thing + "_seed", dimensions, version, type);
    add_string_scalar(parent, thing + "_orientation", (transposed ? std::string("T") : std::string("N")));
}

/**************************************/

class MatrixProductPassTest : public ::testing::TestWithParam<std::tuple<ritsuko::Version, bool> > {};

TEST_P(MatrixProductPassTest, Untransposed) {
    auto path = define_test_path("matrix_product");
    auto params = GetParam();
    auto version = std::get<0>(params);
    auto deets = std::get<1>(params);

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = matrix_product_opener(fhandle, "foos", version);
        add_seed(ghandle, { 10, 20 }, version, "FLOAT", /* left = */ true, /* transposed = */ false);
        add_seed(ghandle, { 20, 15 }, version, "FLOAT", /* left = */ false, /* transposed = */ false);
    }

    auto output = test_validate(path, "foos", deets); 
    EXPECT_EQ(output.type, chihaya::FLOAT);
    EXPECT_EQ(output.dimensions[0], 10);
    EXPECT_EQ(output.dimensions[1], 15);
}

TEST_P(MatrixProductPassTest, OneTransposed) {
    auto path = define_test_path("matrix_product");
    auto params = GetParam();
    auto version = std::get<0>(params);
    auto deets = std::get<1>(params);

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = matrix_product_opener(fhandle, "foos", version);
        add_seed(ghandle, { 10, 20 }, version, "FLOAT", /* left = */ true, /* transposed = */ true);
        add_seed(ghandle, { 10, 15 }, version, "INTEGER", /* left = */ false, /* transposed = */ false);
    }
    {
        auto output = test_validate(path, "foos", deets);
        EXPECT_EQ(output.type, chihaya::FLOAT);
        EXPECT_EQ(output.dimensions[0], 20);
        EXPECT_EQ(output.dimensions[1], 15);
    }

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = matrix_product_opener(fhandle, "foos", version);
        add_seed(ghandle, { 10, 20 }, version, "INTEGER", /* left = */ true, /* transposed = */ false);
        add_seed(ghandle, { 30, 20 }, version, "FLOAT", /* left = */ false, /* transposed = */ true);
    }
    {
        auto output = test_validate(path, "foos", deets);
        EXPECT_EQ(output.type, chihaya::FLOAT);
        EXPECT_EQ(output.dimensions[0], 10);
        EXPECT_EQ(output.dimensions[1], 30);
    }
}

TEST_P(MatrixProductPassTest, BothTransposed) {
    auto path = define_test_path("matrix_product");
    auto params = GetParam();
    auto version = std::get<0>(params);
    auto deets = std::get<1>(params);

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = matrix_product_opener(fhandle, "foos", version);
        add_seed(ghandle, { 20, 10 }, version, "INTEGER", /* left = */ true, /* transposed = */ true);
        add_seed(ghandle, { 15, 20 }, version, "INTEGER", /* left = */ false, /* transposed = */ true);
    }

    auto output = test_validate(path, "foos", deets); 
    EXPECT_EQ(output.type, chihaya::INTEGER); // If both operands are integer, the result is also considered to be INTEGER.
    EXPECT_EQ(output.dimensions[0], 10);
    EXPECT_EQ(output.dimensions[1], 15);
}

INSTANTIATE_TEST_SUITE_P(
    MatrixProduct,
    MatrixProductPassTest,
    ::testing::Combine(
        spawn_all_versions(),
        ::testing::Values(false, true)
    )
);

/**************************************/

class MatrixProductErrorTest : public ::testing::TestWithParam<ritsuko::Version> {};

TEST_P(MatrixProductErrorTest, Seed) {
    auto path = define_test_path("matrix_product");
    auto version = GetParam();

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = matrix_product_opener(fhandle, "foos", version);
        add_seed(ghandle, { 10, 20 }, version, "FLOAT", /* left = */ true, /* transposed = */ false);
        add_seed(ghandle, { 10, 20, 30 }, version, "FLOAT", /* left = */ false, /* transposed = */ false);
    }
    expect_error(path, "foos", "2-dimensional");

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = matrix_product_opener(fhandle, "foos", version);
        add_seed(ghandle, { 10, 20 }, version, "FLOAT", /* left = */ true, /* transposed = */ false);
        add_seed(ghandle, { 20, 10 }, version, "STRING", /* left = */ false, /* transposed = */ false);
    }
    expect_error(path, "foos", "integer, float or boolean");
}

TEST_P(MatrixProductErrorTest, Orientation) {
    auto path = define_test_path("matrix_product");
    auto version = GetParam();

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = matrix_product_opener(fhandle, "foos", version);
        add_seed(ghandle, { 10, 20 }, version, "FLOAT", /* left = */ true, /* transposed = */ false);
        add_seed(ghandle, { 20, 10 }, version, "FLOAT", /* left = */ false, /* transposed = */ false);
        ghandle.unlink("left_orientation");
        add_numeric_scalar<int>(ghandle, "left_orientation", 10, H5::PredType::NATIVE_INT);
    }
    expect_error(path, "foos", "UTF-8 encoded string");

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = matrix_product_opener(fhandle, "foos", version);
        add_seed(ghandle, { 10, 20 }, version, "FLOAT", /* left = */ true, /* transposed = */ false);
        add_seed(ghandle, { 20, 10 }, version, "FLOAT", /* left = */ false, /* transposed = */ false);
        ghandle.unlink("right_orientation");
        add_string_vector(ghandle, "right_orientation", 2);
    }
    expect_error(path, "foos", "scalar");

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = matrix_product_opener(fhandle, "foos", version);
        add_seed(ghandle, { 10, 20 }, version, "FLOAT", /* left = */ true, /* transposed = */ false);
        add_seed(ghandle, { 20, 10 }, version, "FLOAT", /* left = */ false, /* transposed = */ false);
        ghandle.unlink("left_orientation");
        add_string_scalar(ghandle, "left_orientation", "FOO");
    }
    expect_error(path, "foos", "'left_orientation' should be either 'N' or 'T'");
}

TEST_P(MatrixProductErrorTest, Dimensions) {
    auto path = define_test_path("matrix_product");
    auto version = GetParam();

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = matrix_product_opener(fhandle, "foos", version);
        add_seed(ghandle, { 10, 20 }, version, "FLOAT", /* left = */ true, /* transposed = */ false);
        add_seed(ghandle, { 15, 10 }, version, "FLOAT", /* left = */ false, /* transposed = */ false);
    }
    expect_error(path, "foos", "inconsistent common dimensions");
}

INSTANTIATE_TEST_SUITE_P(
    MatrixProduct,
    MatrixProductErrorTest,
    spawn_all_versions()
);
