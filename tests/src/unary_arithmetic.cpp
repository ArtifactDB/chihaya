#include <gtest/gtest.h>

#include <string>
#include <vector>
#include <cstddef>

#include "H5Cpp.h"
#include "ritsuko/ritsuko.hpp"
#include "chihaya/unary_arithmetic.hpp"

#include "utils.h"

static H5::Group unary_arithmetic_opener(
    H5::Group& handle,
    const std::string& name,
    const std::string& method,
    const std::string& side,
    const std::vector<std::size_t>& dimensions,
    const ritsuko::Version& version,
    const std::string& type
) {
    auto ghandle = operation_opener(handle, name, "unary arithmetic");
    add_version_string(ghandle, version);
    mock_array_opener(ghandle, "seed", dimensions, version, type);
    add_string_scalar(ghandle, "method", method);
    add_string_scalar(ghandle, "side", side);
    return ghandle;
}

/**********************************/

class UnaryArithmeticPassTest : public ::testing::TestWithParam<std::tuple<ritsuko::Version, bool> > {};

TEST_P(UnaryArithmeticPassTest, PureUnary) {
    auto path = define_test_path("unary_arithmetic");
    auto params = GetParam();
    auto version = std::get<0>(params);
    auto deets = std::get<1>(params);

    std::vector<std::size_t> dims{ 17, 21 };
    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        unary_arithmetic_opener(fhandle, "hello", "+", "none", dims, version, "INTEGER");
    }
    {
        auto output = test_validate(path, "hello", deets); 
        EXPECT_EQ(output.type, chihaya::INTEGER);
        EXPECT_EQ(output.dimensions, dims);
    }

    // Checking negation as well.
    // Note that any unary arithmetic operation on booleans is considered to yield an integer.
    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        unary_arithmetic_opener(fhandle, "hello", "-", "none", dims, version, "BOOLEAN");
    }
    {
        auto output = test_validate(path, "hello", deets); 
        EXPECT_EQ(output.type, chihaya::INTEGER);
        EXPECT_EQ(output.dimensions, dims);
    }
}

TEST_P(UnaryArithmeticPassTest, Scalar) {
    auto path = define_test_path("unary_arithmetic");
    auto params = GetParam();
    auto version = std::get<0>(params);
    auto deets = std::get<1>(params);

    std::vector<std::size_t> dims { 23, 11 };
    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = unary_arithmetic_opener(fhandle, "hello", "-", "left", dims, version, "INTEGER");
        auto dhandle = add_numeric_scalar<double>(ghandle, "value", 2.5, H5::PredType::NATIVE_DOUBLE);
        if (version.ge(1, 1, 0)) {
            add_string_attribute(dhandle, "type", "FLOAT");
        }
    }

    auto output = test_validate(path, "hello", deets); 
    EXPECT_EQ(output.type, chihaya::FLOAT);
    EXPECT_EQ(output.dimensions, dims);
}

TEST_P(UnaryArithmeticPassTest, Vector) {
    auto path = define_test_path("unary_arithmetic");
    auto params = GetParam();
    auto version = std::get<0>(params);
    auto deets = std::get<1>(params);

    std::vector<std::size_t> dims { 5, 19 };
    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = unary_arithmetic_opener(fhandle, "hello", "/", "left", dims, version, "INTEGER");
        auto dhandle = add_numeric_vector<int>(ghandle, "value", { 1, 2, 3, 4, 5 }, H5::PredType::NATIVE_INT16);
        if (version.ge(1, 1, 0)) {
            add_string_attribute(dhandle, "type", "INTEGER");
            add_numeric_scalar<int>(ghandle, "along", 0, H5::PredType::NATIVE_UINT8);
        } else {
            add_numeric_scalar<int>(ghandle, "along", 0, H5::PredType::NATIVE_INT32);
        }
    }

    auto output = test_validate(path, "hello", deets); 
    EXPECT_EQ(output.type, chihaya::FLOAT);
    EXPECT_EQ(output.dimensions, dims);
}

TEST_P(UnaryArithmeticPassTest, Missing) {
    auto path = define_test_path("unary_arithmetic");
    auto params = GetParam();
    auto version = std::get<0>(params);
    auto deets = std::get<1>(params);

    if (version.lt(1, 0, 0)) {
        return;
    }

    std::vector<std::size_t> dims{ 10, 7 };
    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = unary_arithmetic_opener(fhandle, "hello", "*", "right", dims, version, "FLOAT");
        auto dhandle = add_numeric_vector<int>(ghandle, "value", { 6, 5, 4, 3, 2, 1, 0 }, H5::PredType::NATIVE_INT16);
        if (version.ge(1, 1, 0)) {
            add_string_attribute(dhandle, "type", "INTEGER");
            add_numeric_scalar<int>(ghandle, "along", 1, H5::PredType::NATIVE_UINT8);
        } else {
            add_numeric_scalar<int>(ghandle, "along", 1, H5::PredType::NATIVE_INT32);
        }
        add_numeric_missing_placeholder<int>(dhandle, 2, H5::PredType::NATIVE_INT16);
    }

    auto output = test_validate(path, "hello", deets);
    EXPECT_EQ(output.type, chihaya::FLOAT);
    EXPECT_EQ(output.dimensions, dims);
}

INSTANTIATE_TEST_SUITE_P(
    UnaryArithmetic,
    UnaryArithmeticPassTest,
    ::testing::Combine(
        spawn_all_versions(),
        ::testing::Values(false, true)
    )
);

/**********************************/

class UnaryArithmeticErrorTest : public ::testing::TestWithParam<ritsuko::Version> {};

TEST_P(UnaryArithmeticErrorTest, Seed) {
    auto path = define_test_path("unary_arithmetic");
    auto version = GetParam();

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        unary_arithmetic_opener(fhandle, "hello", "*", "right", { 10, 7 }, version, "STRING");
    }
    expect_error(path, "hello", "integer, float or boolean");
}

TEST_P(UnaryArithmeticErrorTest, Method) {
    auto path = define_test_path("unary_arithmetic");
    auto version = GetParam();

    // Test that we actually check for a scalar string dataset.
    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = unary_arithmetic_opener(fhandle, "hello", "*", "right", { 10, 7 }, version, "FLOAT");
        ghandle.unlink("method");
        add_numeric_scalar<int>(ghandle, "method", 1, H5::PredType::NATIVE_INT32);
    }
    expect_error(path, "hello", "UTF-8 encoded string");

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = unary_arithmetic_opener(fhandle, "hello", "*", "right", { 10, 7 }, version, "FLOAT");
        ghandle.unlink("method");
        add_string_scalar(ghandle, "method", "foo");
    }
    expect_error(path, "hello", "unrecognized operation in 'method' (got 'foo')");
}

TEST_P(UnaryArithmeticErrorTest, Side) {
    auto path = define_test_path("unary_arithmetic");
    auto version = GetParam();

    // Test that we actually check for a scalar string dataset.
    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = unary_arithmetic_opener(fhandle, "hello", "*", "right", { 10, 7 }, version, "FLOAT");
        ghandle.unlink("side");
        add_numeric_scalar<int>(ghandle, "side", 1, H5::PredType::NATIVE_INT16);
    }
    expect_error(path, "hello", "UTF-8 encoded string");

    {
        H5::H5File fhandle(path, H5F_ACC_RDWR);
        auto ghandle = fhandle.openGroup("hello");
        ghandle.unlink("side");
        add_string_scalar(ghandle, "side", "foo");
    }
    expect_error(path, "hello", "should be 'left' or 'right'");

    {
        H5::H5File fhandle(path, H5F_ACC_RDWR);
        auto ghandle = fhandle.openGroup("hello");
        ghandle.unlink("side");
        add_string_scalar(ghandle, "side", "none");
    }
    expect_error(path, "hello", "cannot be 'none'");
}

TEST_P(UnaryArithmeticErrorTest, Value) {
    auto path = define_test_path("unary_arithmetic");
    auto version = GetParam();

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = unary_arithmetic_opener(fhandle, "hello", "*", "right", { 10, 7 }, version, "FLOAT");
        auto dhandle = add_string_scalar(ghandle, "value", "WHEE");
        if (version.ge(1, 1, 0)) {
            add_string_attribute(dhandle, "type", "STRING");
        }
    }
    expect_error(path, "hello", "should be integer, float or boolean");

    if (version.ge(1, 1, 0)) {
        // Test that we actually check for a scalar 'type'.
        {
            H5::H5File fhandle(path, H5F_ACC_TRUNC);
            auto ghandle = unary_arithmetic_opener(fhandle, "hello", "*", "right", { 10, 7 }, version, "INTEGER");
            auto dhandle = add_numeric_vector<int>(ghandle, "value", { 1, 2, 3, 4, 5 }, H5::PredType::NATIVE_UINT8);
            constexpr hsize_t one = 1;
            dhandle.createAttribute("type", H5::StrType(0, H5T_VARIABLE), H5::DataSpace(1, &one));
        }
        expect_error(path, "hello", "scalar");

        // Test that we actually check the 'type' is consistent with the dataset's type.
        {
            H5::H5File fhandle(path, H5F_ACC_TRUNC);
            auto ghandle = unary_arithmetic_opener(fhandle, "hello", "*", "right", { 10, 7 }, version, "INTEGER");
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
        auto dhandle = ghandle.createDataSet("value", H5::PredType::NATIVE_INT, H5::DataSpace(2, dims));
        if (version.ge(1, 1, 0)) {
            add_string_attribute(dhandle, "type", "INTEGER");
        }
    }
    expect_error(path, "hello", "dataset should be scalar or 1-dimensional");
}

TEST_P(UnaryArithmeticErrorTest, Along) {
    auto path = define_test_path("unary_arithmetic");
    auto version = GetParam();

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = unary_arithmetic_opener(fhandle, "hello", "/", "left", { 19, 4 }, version, "INTEGER");
        auto dhandle = add_numeric_vector<int>(ghandle, "value", { 1, 2, 3, 4 }, H5::PredType::NATIVE_INT32);
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
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = unary_arithmetic_opener(fhandle, "hello", "/", "left", { 19, 4 }, version, "INTEGER");
        auto dhandle = add_numeric_vector<int>(ghandle, "value", { 1, 2, 3, 4 }, H5::PredType::NATIVE_INT32);
        if (version.ge(1, 1, 0)) {
            add_string_attribute(dhandle, "type", "INTEGER");
        }
        add_numeric_scalar<int>(ghandle, "along", 0, H5::PredType::NATIVE_UINT32);
    }
    expect_error(path, "hello", "should be equal to the dimension");
}

TEST_P(UnaryArithmeticErrorTest, Missing) {
    auto path = define_test_path("unary_arithmetic");
    auto version = GetParam();

    if (version.lt(1, 0, 0)) {
        return;
    }

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = unary_arithmetic_opener(fhandle, "hello", "^", "left", { 5, 19 }, version, "FLOAT");
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
    UnaryArithmetic,
    UnaryArithmeticErrorTest,
    spawn_all_versions()
);
