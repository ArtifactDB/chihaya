#include <gtest/gtest.h>

#include "H5Cpp.h"
#include "ritsuko/ritsuko.hpp"
#include "chihaya/utils_nary.hpp"

#include "utils.h"

class FetchNumericSeedTest : public ::testing::TestWithParam<ritsuko::Version> {};

TEST_P(FetchNumericSeedTest, Basic) {
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

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        mock_array_opener(fhandle, "seed", dimensions, version, "STRING");
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
