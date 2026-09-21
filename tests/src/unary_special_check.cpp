#include <gtest/gtest.h>

#include <vector>
#include <string>
#include <cstddef>

#include "H5Cpp.h"
#include "ritsuko/ritsuko.hpp"
#include "chihaya/unary_special_check.hpp"

#include "utils.h"

static H5::Group unary_special_check_opener(
    H5::Group& handle,
    const std::string& name,
    const std::string& method,
    const std::vector<std::size_t>& dimensions,
    const ritsuko::Version& version,
    const std::string& type
) {
    auto ghandle = operation_opener(handle, name, "unary special check");
    add_version_string(ghandle, version);
    mock_array_opener(ghandle, "seed", dimensions, version, type);
    add_string_scalar(ghandle, "method", method);
    return ghandle;
}

/****************************************/

class UnarySpecialCheckPassTest : public ::testing::TestWithParam<std::tuple<ritsuko::Version, bool> > {};

TEST_P(UnarySpecialCheckPassTest, Basic) {
    auto path = define_test_path("unary_special_check");
    auto params = GetParam();
    auto version = std::get<0>(params);
    auto deets = std::get<1>(params);

    std::vector<std::size_t> dims{ 13, 19 };
    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        unary_special_check_opener(fhandle, "hello", "is_nan", dims, version, "INTEGER");
    }

    auto output = test_validate(path, "hello", deets); 
    EXPECT_EQ(output.type, chihaya::BOOLEAN);
    EXPECT_EQ(output.dimensions, dims);
}

INSTANTIATE_TEST_SUITE_P(
    UnarySpecialCheck,
    UnarySpecialCheckPassTest,
    ::testing::Combine(
        spawn_all_versions(),
        ::testing::Values(false, true)
    )
);

/****************************************/

class UnarySpecialCheckErrorTest : public ::testing::TestWithParam<ritsuko::Version> {};

TEST_P(UnarySpecialCheckErrorTest, Seed) {
    auto path = define_test_path("unary_special_check");
    auto version = GetParam();

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        unary_special_check_opener(fhandle, "hello", "is_nan", { 13, 19 }, version, "STRING");
    }
    expect_error(path, "hello", "integer, float or boolean");
}

TEST_P(UnarySpecialCheckErrorTest, Method) {
    auto path = define_test_path("unary_special_check");
    auto version = GetParam();

    // Test that we actually test for a scalar string 'method'.
    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = unary_special_check_opener(fhandle, "hello", "is_nan", { 13, 19 }, version, "FLOAT");
        ghandle.unlink("method");
        add_numeric_scalar(ghandle, "method", 2, H5::PredType::NATIVE_INT);
    }
    expect_error(path, "hello", "UTF-8 encoded string");

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = unary_special_check_opener(fhandle, "hello", "is_nan", { 13, 19 }, version, "FLOAT");
        ghandle.unlink("method");
        add_string_scalar(ghandle, "method", "foo");
    }
    expect_error(path, "hello", "unrecognized 'method'");
}

INSTANTIATE_TEST_SUITE_P(
    UnarySpecialCheck,
    UnarySpecialCheckErrorTest,
    spawn_all_versions()
);
