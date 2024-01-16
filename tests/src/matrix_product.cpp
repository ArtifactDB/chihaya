#include <gtest/gtest.h>
#include "chihaya/chihaya.hpp"
#include "utils.h"

class MatrixProductTest : public ::testing::TestWithParam<int> {
public:
    MatrixProductTest() : path("Test_product.h5") {}
protected:
    std::string path;

    static H5::Group matrix_product_opener(H5::Group& handle, const std::string& name, int version) {
        auto ghandle = operation_opener(handle, name, "matrix product");
        add_version_string(ghandle, version);
        return ghandle;
    }

    template<bool left>
    static void add_seed(H5::Group& parent, const std::vector<int>& dimensions, int version, std::string type, bool transposed) {
        std::string thing = (left ? std::string("left") : std::string("right"));
        mock_array_opener(parent, thing + "_seed", dimensions, version, type);
        add_string_scalar(parent, thing + "_orientation", (transposed ? std::string("T") : std::string("N")));
    }
};

TEST_P(MatrixProductTest, Simple) {
    auto version = GetParam();

    // Both untransposed.
    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = matrix_product_opener(fhandle, "foos", version);
        add_seed<true>(ghandle, { 10, 20 }, version, "FLOAT", false);
        add_seed<false>(ghandle, { 20, 15 }, version, "FLOAT", false);
    }
    {
        auto output = test_validate(path, "foos"); 
        EXPECT_EQ(output.type, chihaya::FLOAT);
        EXPECT_EQ(output.dimensions[0], 10);
        EXPECT_EQ(output.dimensions[1], 15);

        auto skipped = test_validate_skip(path, "foos");
        EXPECT_EQ(skipped.type, output.type);
        EXPECT_EQ(skipped.dimensions, output.dimensions);
    }

    // One or the other transposed.
    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = matrix_product_opener(fhandle, "foos", version);
        add_seed<true>(ghandle, { 10, 20 }, version, "FLOAT", true);
        add_seed<false>(ghandle, { 10, 15 }, version, "INTEGER", false);
    }
    {
        auto output = test_validate(path, "foos"); 
        EXPECT_EQ(output.type, chihaya::FLOAT);
        EXPECT_EQ(output.dimensions[0], 20);
        EXPECT_EQ(output.dimensions[1], 15);
    }

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = matrix_product_opener(fhandle, "foos", version);
        add_seed<true>(ghandle, { 10, 20 }, version, "INTEGER", false);
        add_seed<false>(ghandle, { 30, 20 }, version, "FLOAT", true);
    }
    {
        auto output = test_validate(path, "foos"); 
        EXPECT_EQ(output.type, chihaya::FLOAT);
        EXPECT_EQ(output.dimensions[0], 10);
        EXPECT_EQ(output.dimensions[1], 30);
    }

    // Both transposed.
    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = matrix_product_opener(fhandle, "foos", version);
        add_seed<true>(ghandle, { 20, 10 }, version, "INTEGER", true);
        add_seed<false>(ghandle, { 15, 20 }, version, "INTEGER", true);
    }
    {
        auto output = test_validate(path, "foos"); 
        EXPECT_EQ(output.type, chihaya::INTEGER);
        EXPECT_EQ(output.dimensions[0], 10);
        EXPECT_EQ(output.dimensions[1], 15);
    }
}

TEST_P(MatrixProductTest, SeedErrors) {
    auto version = GetParam();

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = matrix_product_opener(fhandle, "foos", version);
        add_seed<true>(ghandle, { 10, 20 }, version, "FLOAT", false);
    }
    expect_error(path, "foos", "expected a group at 'right_seed'");

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = matrix_product_opener(fhandle, "foos", version);
        add_seed<true>(ghandle, { 10, 20 }, version, "FLOAT", false);
        add_seed<false>(ghandle, { 10, 20, 30 }, version, "FLOAT", false);
    }
    expect_error(path, "foos", "2-dimensional");

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = matrix_product_opener(fhandle, "foos", version);
        add_seed<true>(ghandle, { 10, 20 }, version, "FLOAT", false);
        add_seed<false>(ghandle, { 20, 10 }, version, "STRING", false);
    }
    expect_error(path, "foos", "integer, float or boolean");
}

TEST_P(MatrixProductTest, OrientationErrors) {
    auto version = GetParam();

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = matrix_product_opener(fhandle, "foos", version);
        add_seed<true>(ghandle, { 10, 20 }, version, "FLOAT", false);
        ghandle.unlink("left_orientation");
    }
    expect_error(path, "foos", "expected a dataset at 'left_orientation'");

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = matrix_product_opener(fhandle, "foos", version);
        add_seed<true>(ghandle, { 10, 20 }, version, "FLOAT", false);
        ghandle.unlink("left_orientation");
        add_numeric_scalar<int>(ghandle, "left_orientation", 10, H5::PredType::NATIVE_INT);
    }
    expect_error(path, "foos", "UTF-8 encoded string");

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = matrix_product_opener(fhandle, "foos", version);
        add_seed<true>(ghandle, { 10, 20 }, version, "FLOAT", false);
        ghandle.unlink("left_orientation");
        add_string_scalar(ghandle, "left_orientation", "FOO");
    }
    expect_error(path, "foos", "'left_orientation' should be either 'N' or 'T'");

    // Checking the combination.
    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = matrix_product_opener(fhandle, "foos", version);
        add_seed<true>(ghandle, { 10, 20 }, version, "FLOAT", false);
        add_seed<false>(ghandle, { 10, 15 }, version, "FLOAT", false);
    }
    expect_error(path, "foos", "inconsistent common dimensions");
}

INSTANTIATE_TEST_SUITE_P(
    MatrixProduct,
    MatrixProductTest,
    ::testing::Values(0, 1000000, 1100000)
);
