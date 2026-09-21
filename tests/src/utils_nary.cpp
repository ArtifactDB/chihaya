#include <gtest/gtest.h>

#include "H5Cpp.h"
#include "ritsuko/ritsuko.hpp"
#include "chihaya/utils_nary.hpp"

#include "utils.h"

class FetchNumericSeedTest : public ::testing::TestWithParam<ritsuko::Version> {};

TEST_P(FetchNumericSeedTest, Okay) {
    auto version = GetParam();
    auto path = define_test_path("utils_nary");
    chihaya::Options options;

    std::vector<std::size_t> dimensions{ 13, 14 };
    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        mock_array_opener(fhandle, "seed", dimensions, version, "FLOAT");
    }
    auto deets = chihaya::fetch_numeric_seed(H5::H5File(path, H5F_ACC_RDONLY), "seed", version, options); 
    EXPECT_EQ(deets.type, chihaya::FLOAT);
    EXPECT_EQ(deets.dimensions, dimensions);
}

TEST_P(FetchNumericSeedTest, Error) {
    auto version = GetParam();
    auto path = define_test_path("utils_nary");
    chihaya::Options options;

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        mock_array_opener<int>(fhandle, "seed", { 42, 11 }, version, "STRING");
    }
    expect_error(
        [&]() -> void {
            chihaya::fetch_numeric_seed(H5::H5File(path, H5F_ACC_RDONLY), "seed", version, options); 
        },
        "should be integer, float or boolean"
    );
}

INSTANTIATE_TEST_SUITE_P(
    FetchNumericSeed,
    FetchNumericSeedTest,
    spawn_all_versions()
);

/*****************************/

TEST(IsValidOperation, Arithmetic) {
    EXPECT_TRUE(chihaya::is_valid_arithmetic_operation("%%"));
    EXPECT_TRUE(chihaya::is_valid_arithmetic_operation("-"));
    EXPECT_TRUE(chihaya::is_valid_arithmetic_operation("+"));
    EXPECT_TRUE(chihaya::is_valid_arithmetic_operation("/"));
    EXPECT_TRUE(chihaya::is_valid_arithmetic_operation("*"));
    EXPECT_TRUE(chihaya::is_valid_arithmetic_operation("%/%"));
    EXPECT_TRUE(chihaya::is_valid_arithmetic_operation("^"));
    EXPECT_FALSE(chihaya::is_valid_arithmetic_operation("foo"));
}

TEST(IsValidOperation, Comparison) {
    EXPECT_TRUE(chihaya::is_valid_comparison_operation("=="));
    EXPECT_TRUE(chihaya::is_valid_comparison_operation(">="));
    EXPECT_TRUE(chihaya::is_valid_comparison_operation(">"));
    EXPECT_TRUE(chihaya::is_valid_comparison_operation("<"));
    EXPECT_TRUE(chihaya::is_valid_comparison_operation("<="));
    EXPECT_TRUE(chihaya::is_valid_comparison_operation("!="));
    EXPECT_FALSE(chihaya::is_valid_comparison_operation("bar"));
}

TEST(IsValidOperation, Logic) {
    EXPECT_TRUE(chihaya::is_valid_logic_operation("&&"));
    EXPECT_TRUE(chihaya::is_valid_logic_operation("||"));
    EXPECT_FALSE(chihaya::is_valid_logic_operation("stuff"));
}

TEST(IsValidOperation, Math) {
    EXPECT_TRUE(chihaya::is_other_math("log1p"));
    EXPECT_TRUE(chihaya::is_other_math("sqrt"));
    EXPECT_TRUE(chihaya::is_other_math("exp"));
    EXPECT_TRUE(chihaya::is_other_math("expm1"));
    EXPECT_TRUE(chihaya::is_other_math("ceiling"));
    EXPECT_TRUE(chihaya::is_other_math("floor"));
    EXPECT_TRUE(chihaya::is_other_math("trunc"));
    EXPECT_TRUE(chihaya::is_other_math("sin"));
    EXPECT_TRUE(chihaya::is_other_math("cos"));
    EXPECT_TRUE(chihaya::is_other_math("tan"));
    EXPECT_TRUE(chihaya::is_other_math("acos"));
    EXPECT_TRUE(chihaya::is_other_math("asin"));
    EXPECT_TRUE(chihaya::is_other_math("atan"));
    EXPECT_TRUE(chihaya::is_other_math("sinh"));
    EXPECT_TRUE(chihaya::is_other_math("cosh"));
    EXPECT_TRUE(chihaya::is_other_math("tanh"));
    EXPECT_TRUE(chihaya::is_other_math("acosh"));
    EXPECT_TRUE(chihaya::is_other_math("asinh"));
    EXPECT_TRUE(chihaya::is_other_math("atanh"));
    EXPECT_FALSE(chihaya::is_other_math("foo"));
}

TEST(IsValidOperation, SpecialCheck) {
    EXPECT_TRUE(chihaya::is_valid_special_check_operation("is_nan"));
    EXPECT_TRUE(chihaya::is_valid_special_check_operation("is_finite"));
    EXPECT_TRUE(chihaya::is_valid_special_check_operation("is_infinite"));
    EXPECT_FALSE(chihaya::is_valid_special_check_operation("foo"));
}

/*****************************/

class CheckUnaryAlongTest : public ::testing::TestWithParam<ritsuko::Version> {};

TEST_P(CheckUnaryAlongTest, Okay) {
    auto version = GetParam();
    auto path = define_test_path("utils_nary");

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        add_numeric_scalar(fhandle, "along", 0, H5::PredType::NATIVE_UINT8);
    }
    {
        H5::H5File fhandle(path, H5F_ACC_RDONLY);
        chihaya::check_unary_along(fhandle, version, std::vector<std::size_t>{ 10, 20 }, 10);
    }

    // More dimensions.
    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        add_numeric_scalar(fhandle, "along", 2, H5::PredType::NATIVE_UINT8);
    }
    {
        H5::H5File fhandle(path, H5F_ACC_RDONLY);
        chihaya::check_unary_along(fhandle, version, std::vector<std::size_t>{ 10, 20, 30 }, 30);
    }
}

TEST_P(CheckUnaryAlongTest, Error) {
    auto version = GetParam();
    auto path = define_test_path("utils_nary");

    // Check that 'load_along' is actually called and checks for a scalar integer.
    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        add_numeric_vector<int>(fhandle, "along", { 1, 2, 3 }, H5::PredType::NATIVE_UINT8);
    }
    {
        H5::H5File fhandle(path, H5F_ACC_RDONLY);
        expect_error([&]() -> void {
            chihaya::check_unary_along(fhandle, version, std::vector<std::size_t>{ 10, 20 }, 10);
        }, "scalar");
    }

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        add_numeric_scalar(fhandle, "along", 2, H5::PredType::NATIVE_UINT8);
    }
    {
        H5::H5File fhandle(path, H5F_ACC_RDONLY);
        expect_error([&]() -> void {
            chihaya::check_unary_along(fhandle, version, std::vector<std::size_t>{ 10, 20 }, 10);
        }, "less than the seed dimensionality");
    }

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        add_numeric_scalar(fhandle, "along", 1, H5::PredType::NATIVE_UINT8);
    }
    {
        H5::H5File fhandle(path, H5F_ACC_RDONLY);
        expect_error([&]() -> void {
            chihaya::check_unary_along(fhandle, version, std::vector<std::size_t>{ 10, 20 }, 15);
        }, "equal to the dimension");
    }
}

INSTANTIATE_TEST_SUITE_P(
    CheckUnaryAlong,
    CheckUnaryAlongTest,
    spawn_all_versions()
);
