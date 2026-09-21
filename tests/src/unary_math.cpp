#include <gtest/gtest.h>

#include <string>
#include <vector>
#include <cstddef>

#include "H5Cpp.h"
#include "ritsuko/ritsuko.hpp"
#include "chihaya/unary_math.hpp"

#include "utils.h"

static H5::Group unary_math_opener(
    H5::Group& handle,
    const std::string& name,
    const std::string& method,
    const std::vector<std::size_t>& dimensions,
    const ritsuko::Version& version,
    const std::string& type
) {
    auto ghandle = operation_opener(handle, name, "unary math");
    add_version_string(ghandle, version);
    mock_array_opener(ghandle, "seed", dimensions, version, type);
    add_string_scalar(ghandle, "method", method);
    return ghandle;
}

/**********************************/

class UnaryMathPassTest : public ::testing::TestWithParam<std::tuple<ritsuko::Version, bool> > {};

TEST_P(UnaryMathPassTest, PureUnary) {
    auto path = define_test_path("unary_math");
    auto param = GetParam();
    auto version = std::get<0>(param);
    auto deets = std::get<1>(param);

    std::vector<std::size_t> dims{ 42, 18 };
    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        unary_math_opener(fhandle, "hello", "abs", dims, version, "INTEGER");
    }
    {
        auto output = test_validate(path, "hello", deets); 
        EXPECT_EQ(output.type, chihaya::INTEGER);
        EXPECT_EQ(output.dimensions, dims);
    }

    // Different type for sign.
    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        unary_math_opener(fhandle, "hello", "sign", dims, version, "FLOAT");
    }
    {
        auto output = test_validate(path, "hello", deets);
        EXPECT_EQ(output.type, chihaya::INTEGER);
        EXPECT_EQ(output.dimensions, dims);
    }

    // Different type for log1p and related operations.
    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        unary_math_opener(fhandle, "hello", "log1p", dims, version, "BOOLEAN");
    }
    {
        auto output = test_validate(path, "hello", deets);
        EXPECT_EQ(output.type, chihaya::FLOAT);
        EXPECT_EQ(output.dimensions, dims);
    }
}

TEST_P(UnaryMathPassTest, LogBase) {
    auto path = define_test_path("unary_math");
    auto param = GetParam();
    auto version = std::get<0>(param);
    auto deets = std::get<1>(param);

    std::vector<std::size_t> dims{ 14, 27 };
    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = unary_math_opener(fhandle, "hello", "log", dims, version, "INTEGER");
        add_numeric_scalar<double>(ghandle, "base", 2, H5::PredType::NATIVE_DOUBLE);
    }

    auto output = test_validate(path, "hello", deets); 
    EXPECT_EQ(output.type, chihaya::FLOAT);
    EXPECT_EQ(output.dimensions, dims);
}

TEST_P(UnaryMathPassTest, RoundDigits) {
    auto path = define_test_path("unary_math");
    auto param = GetParam();
    auto version = std::get<0>(param);
    auto deets = std::get<1>(param);

    std::vector<std::size_t> dims{ 9, 81 };
    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = unary_math_opener(fhandle, "hello", "round", dims, version, "FLOAT");
        add_numeric_scalar<int>(ghandle, "digits", 2, H5::PredType::NATIVE_INT32);
    }

    auto output = test_validate(path, "hello", deets); 
    EXPECT_EQ(output.type, chihaya::FLOAT);
    EXPECT_EQ(output.dimensions, dims);
}

INSTANTIATE_TEST_SUITE_P(
    UnaryMath,
    UnaryMathPassTest,
    ::testing::Combine(
        spawn_all_versions(),
        ::testing::Values(false, true)
    )
);

/**********************************/

class UnaryMathErrorTest : public ::testing::TestWithParam<ritsuko::Version> {};

TEST_P(UnaryMathErrorTest, Seed) {
    auto path = define_test_path("unary_math");
    auto version = GetParam();

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        unary_math_opener(fhandle, "hello", "round", { 10, 7 }, version, "STRING");
    }
    expect_error(path, "hello", "integer, float or boolean");
}

TEST_P(UnaryMathErrorTest, Method) {
    auto path = define_test_path("unary_math");
    auto version = GetParam();

    // Test that we actually check for a scalar string dataset.
    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = unary_math_opener(fhandle, "hello", "sin", { 5, 12 }, version, "FLOAT");
        ghandle.unlink("method");
        add_numeric_scalar<int>(ghandle, "method", 1, H5::PredType::NATIVE_INT);
    }
    expect_error(path, "hello", "UTF-8 encoded string");

    {
        H5::H5File fhandle(path, H5F_ACC_RDWR);
        auto ghandle = fhandle.openGroup("hello");
        ghandle.unlink("method");
        add_string_scalar(ghandle, "method", "foo");
    }
    expect_error(path, "hello", "unrecognized operation");
}

TEST_P(UnaryMathErrorTest, Base) {
    auto path = define_test_path("unary_math");
    auto version = GetParam();

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = unary_math_opener(fhandle, "hello", "log", { 5, 12 }, version, "FLOAT");
        add_string_scalar(ghandle, "base", "foo");
    }
    if (version.lt(1, 1, 0)) {
        expect_error(path, "hello", "'base' should be a float");
    } else {
        expect_error(path, "hello", "64-bit float");
    }
}

TEST_P(UnaryMathErrorTest, Digits) {
    auto path = define_test_path("unary_math");
    auto version = GetParam();

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = unary_math_opener(fhandle, "hello", "signif", { 5, 12 }, version, "FLOAT");
        add_numeric_scalar<double>(ghandle, "digits", 2, H5::PredType::NATIVE_DOUBLE);
    }
    if (version.lt(1, 1, 0)) {
        expect_error(path, "hello", "'digits' should be an integer");
    } else {
        expect_error(path, "hello", "32-bit signed integer");
    }
}

INSTANTIATE_TEST_SUITE_P(
    UnaryMath,
    UnaryMathErrorTest,
    spawn_all_versions()
);
