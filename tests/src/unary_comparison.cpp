#include <gtest/gtest.h>

#include <string>
#include <vector>
#include <cstddef>

#include "chihaya/unary_comparison.hpp"

#include "utils.h"

static H5::Group unary_comparison_opener(
    H5::Group& handle,
    const std::string& name,
    const std::string& method,
    const std::string& side,
    const std::vector<std::size_t>& dimensions,
    const ritsuko::Version& version,
    const std::string& type
) {
    auto ghandle = operation_opener(handle, name, "unary comparison");
    add_version_string(ghandle, version);
    mock_array_opener(ghandle, "seed", dimensions, version, type);
    add_string_scalar(ghandle, "method", method);
    add_string_scalar(ghandle, "side", side);
    return ghandle;
}

/**********************************/

class UnaryComparisonPassTest : public ::testing::TestWithParam<std::tuple<ritsuko::Version, bool> > {};

TEST_P(UnaryComparisonPassTest, Scalar) {
    auto path = define_test_path("unary_comparison");
    auto params = GetParam();
    auto version = std::get<0>(params);
    auto deets = std::get<1>(params);

    std::vector<std::size_t> dims{ 21, 43 };
    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = unary_comparison_opener(fhandle, "hello", "!=", "left", dims, version, "INTEGER");
        auto dhandle = add_numeric_scalar<double>(ghandle, "value", 2.5, H5::PredType::NATIVE_DOUBLE);
        if (version.ge(1, 1, 0)) {
            add_string_attribute(dhandle, "type", "FLOAT");
        }
    }

    auto output = test_validate(path, "hello", deets); 
    EXPECT_EQ(output.type, chihaya::BOOLEAN);
    EXPECT_EQ(output.dimensions, dims);
}

TEST_P(UnaryComparisonPassTest, ScalarStrings) {
    auto path = define_test_path("unary_comparison");
    auto params = GetParam();
    auto version = std::get<0>(params);
    auto deets = std::get<1>(params);

    std::vector<std::size_t> dims{ 30, 15 };
    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = unary_comparison_opener(fhandle, "hello", "!=", "left", dims, version, "STRING");
        auto dhandle = add_string_scalar(ghandle, "value", "FOO");
        if (version.ge(1, 1, 0)) {
            add_string_attribute(dhandle, "type", "STRING");
        }
    }

    auto output = test_validate(path, "hello", deets); 
    EXPECT_EQ(output.type, chihaya::BOOLEAN);
    EXPECT_EQ(output.dimensions, dims);
}

TEST_P(UnaryComparisonPassTest, Vector) {
    auto path = define_test_path("unary_comparison");
    auto params = GetParam();
    auto version = std::get<0>(params);
    auto deets = std::get<1>(params);

    std::vector<std::size_t> dims{ 5, 2 };
    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = unary_comparison_opener(fhandle, "hello", ">", "left", dims, version, "INTEGER");
        add_numeric_scalar<int>(ghandle, "along", 0, H5::PredType::NATIVE_UINT8);
        auto dhandle = add_numeric_vector<int>(ghandle, "value", { 1, 2, 3, 4, 5 }, H5::PredType::NATIVE_INT8);
        if (version.ge(1, 1, 0)) {
            add_string_attribute(dhandle, "type", "INTEGER");
        }
    }

    auto output = test_validate(path, "hello", deets); 
    EXPECT_EQ(output.type, chihaya::BOOLEAN);
    EXPECT_EQ(output.dimensions, dims);
}

TEST_P(UnaryComparisonPassTest, Missing) {
    auto path = define_test_path("unary_comparison");
    auto params = GetParam();
    auto version = std::get<0>(params);
    auto deets = std::get<1>(params);

    if (version.lt(1, 0, 0)) {
        return;
    }

    std::vector<std::size_t> dims{ 5, 2 };
    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = unary_comparison_opener(fhandle, "hello", "==", "right", dims, version, "INTEGER");
        add_numeric_scalar<int>(ghandle, "along", 0, H5::PredType::NATIVE_INT8);
        auto dhandle = add_numeric_scalar<int>(ghandle, "value", 11, H5::PredType::NATIVE_INT8);
        if (version.ge(1, 1, 0)) {
            add_string_attribute(dhandle, "type", "INTEGER");
            add_numeric_missing_placeholder(dhandle, 2, H5::PredType::NATIVE_INT8);
        } else {
            add_numeric_missing_placeholder(dhandle, 2, H5::PredType::NATIVE_INT32);
        }
    }

    auto output = test_validate(path, "hello", deets); 
    EXPECT_EQ(output.type, chihaya::BOOLEAN);
    EXPECT_EQ(output.dimensions, dims);
}

INSTANTIATE_TEST_SUITE_P(
    UnaryComparison,
    UnaryComparisonPassTest,
    ::testing::Combine(
        spawn_all_versions(),
        ::testing::Values(false, true)
    )
);

/**********************************/

class UnaryComparisonErrorTest : public ::testing::TestWithParam<ritsuko::Version> {};

TEST_P(UnaryComparisonErrorTest, Method) {
    auto path = define_test_path("unary_comparison");
    auto version = GetParam();

    // Test that we actually check that 'method' is a scalar string dataset.
    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = unary_comparison_opener(fhandle, "hello", "==", "right", { 15, 12 }, version, "INTEGER");
        ghandle.unlink("method");
        add_numeric_scalar<int>(ghandle, "method", 1, H5::PredType::NATIVE_INT);
    }
    expect_error(path, "hello", "can be represented by a UTF-8 encoded string");

    {
        H5::H5File fhandle(path, H5F_ACC_RDWR);
        auto ghandle = fhandle.openGroup("hello");
        ghandle.unlink("method");
        add_string_scalar(ghandle, "method", "foo");
    }
    expect_error(path, "hello", "unrecognized operation");
}

TEST_P(UnaryComparisonErrorTest, Side) {
    auto path = define_test_path("unary_comparison");
    auto version = GetParam();

    // Test that we actually check that 'side' is a scalar string dataset.
    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = unary_comparison_opener(fhandle, "hello", "==", "right", { 15, 12 }, version, "INTEGER");
        ghandle.unlink("side");
        add_numeric_scalar<int>(ghandle, "side", 1, H5::PredType::NATIVE_INT);
    }
    expect_error(path, "hello", "can be represented by a UTF-8 encoded string");

    {
        H5::H5File fhandle(path, H5F_ACC_RDWR);
        auto ghandle = fhandle.openGroup("hello");
        ghandle.unlink("side");
        add_string_scalar(ghandle, "side", "foo");
    }
    expect_error(path, "hello", "either 'left' or 'right'");
}

TEST_P(UnaryComparisonErrorTest, Value) {
    auto path = define_test_path("unary_comparison");
    auto version = GetParam();

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = unary_comparison_opener(fhandle, "hello", ">=", "right", { 15, 12 }, version, "INTEGER");
        auto dhandle = add_string_scalar(ghandle, "value", "WHEE");
        if (version.ge(1, 1, 0)) {
            add_string_attribute(dhandle, "type", "STRING");
        }
    }
    expect_error(path, "hello", "both or neither");

    if (version.ge(1, 1, 0)) {
        // Test that we actually check for a scalar 'type'.
        {
            H5::H5File fhandle(path, H5F_ACC_TRUNC);
            auto ghandle = unary_comparison_opener(fhandle, "hello", "!=", "right", { 10, 7 }, version, "INTEGER");
            auto dhandle = add_numeric_vector<int>(ghandle, "value", { 1, 2, 3, 4, 5 }, H5::PredType::NATIVE_UINT8);
            constexpr hsize_t one = 1;
            dhandle.createAttribute("type", H5::StrType(0, H5T_VARIABLE), H5::DataSpace(1, &one));
        }
        expect_error(path, "hello", "scalar");

        // Test that we actually check the 'type' is consistent with the dataset's type.
        {
            H5::H5File fhandle(path, H5F_ACC_TRUNC);
            auto ghandle = unary_comparison_opener(fhandle, "hello", "==", "right", { 10, 7 }, version, "INTEGER");
            auto dhandle = add_numeric_vector<int>(ghandle, "value", { 1, 2, 3, 4, 5 }, H5::PredType::NATIVE_INT64);
            add_string_attribute(dhandle, "type", "FLOAT");
        }
        expect_error(path, "hello", "64-bit float");
    }

    {
        H5::H5File fhandle(path, H5F_ACC_RDWR);
        auto ghandle = fhandle.openGroup("hello");
        ghandle.unlink("value");
        hsize_t dims[2] = { 5, 5 };
        auto dhandle = ghandle.createDataSet("value", H5::PredType::NATIVE_INT32, H5::DataSpace(2, dims));
        if (version.ge(1, 1, 0)) {
            add_string_attribute(dhandle, "type", "INTEGER");
        }
    }
    expect_error(path, "hello", "dataset should be scalar or 1-dimensional");
}

TEST_P(UnaryComparisonErrorTest, String) {
    auto path = define_test_path("unary_comparison");
    auto version = GetParam();

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = unary_comparison_opener(fhandle, "hello", ">=", "right", { 15, 12 }, version, "STRING");
        auto dhandle = ghandle.createDataSet("value", H5::StrType(0, H5T_VARIABLE), H5S_SCALAR);
        if (version.ge(1, 1, 0)) {
            add_string_attribute(dhandle, "type", "STRING");
        }
    }
    expect_error(path, "hello", "NULL pointer");

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = unary_comparison_opener(fhandle, "hello", ">=", "right", { 15, 12 }, version, "STRING");
        add_numeric_scalar<int>(ghandle, "along", 0, H5::PredType::NATIVE_UINT8);
        constexpr hsize_t len = 15;
        auto dhandle = ghandle.createDataSet("value", H5::StrType(0, H5T_VARIABLE), H5::DataSpace(1, &len));
        if (version.ge(1, 1, 0)) {
            add_string_attribute(dhandle, "type", "STRING");
        }
    }
    expect_error(path, "hello", "NULL pointer");
}

TEST_P(UnaryComparisonErrorTest, Along) {
    auto path = define_test_path("unary_comparison");
    auto version = GetParam();

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = unary_comparison_opener(fhandle, "hello", "<=", "right", { 21, 4 }, version, "FLOAT");
        auto dhandle = add_numeric_vector<int>(ghandle, "value", { 1, 2, 3, 4 }, H5::PredType::NATIVE_UINT8);
        if (version.ge(1, 1, 0)) {
            add_string_attribute(dhandle, "type", "INTEGER");
        }
        add_string_scalar(ghandle, "along", "WHEE");
    }
    if (version.lt(1, 1, 0)) {
        expect_error(path, "hello", "expected an integer type");
    } else {
        expect_error(path, "hello", "64-bit unsigned integer");
    }

    {
        H5::H5File fhandle(path, H5F_ACC_RDWR);
        auto ghandle = fhandle.openGroup("hello");
        ghandle.unlink("along");
        add_numeric_scalar<int>(ghandle, "along", 0, H5::PredType::NATIVE_UINT8);
    }
    expect_error(path, "hello", "dimension specified in 'along'");
}

TEST_P(UnaryComparisonErrorTest, Missing) {
    auto path = define_test_path("unary_comparison");
    auto version = GetParam();

    if (version.lt(1, 0, 0)) {
        return;
    }

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = unary_comparison_opener(fhandle, "hello", ">", "left", { 5, 19 }, version, "FLOAT");
        auto dhandle = add_numeric_vector<int>(ghandle, "value", { -1, -2, -3, -4 }, H5::PredType::NATIVE_INT32);
        if (version.ge(1, 1, 0)) {
            add_string_attribute(dhandle, "type", "FLOAT");
        }
        add_numeric_missing_placeholder(dhandle, 5, H5::PredType::NATIVE_FLOAT);
    }
    if (version.lt(1, 1, 0)) {
        expect_error(path, "hello", "same datatype class");
    } else {
        expect_error(path, "hello", "same datatype as ");
    }
}

INSTANTIATE_TEST_SUITE_P(
    UnaryComparison,
    UnaryComparisonErrorTest,
    spawn_all_versions()
);
