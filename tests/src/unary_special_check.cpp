#include <gtest/gtest.h>
#include "chihaya/chihaya.hpp"
#include "utils.h"

class UnarySpecialCheckTest : public ::testing::TestWithParam<int> {
public:
    UnarySpecialCheckTest() : path("Test_unary_special_check.h5") {}
protected:
    std::string path;

    static H5::Group unary_special_check_opener(H5::Group& handle, const std::string& name, const std::string& method, const std::vector<int>& dimensions, int version, const std::string& type) {
        auto ghandle = operation_opener(handle, name, "unary special check");
        add_version_string(ghandle, version);
        mock_array_opener(ghandle, "seed", dimensions, version, type);
        add_string_scalar(ghandle, "method", method);
        return ghandle;
    }
};

TEST_P(UnarySpecialCheckTest, Basic) {
    auto version = GetParam();

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        unary_special_check_opener(fhandle, "hello", "is_nan", { 13, 19 }, version, "INTEGER");
    }

    auto output = chihaya::validate(path, "hello"); 
    EXPECT_EQ(output.type, chihaya::BOOLEAN);
    EXPECT_EQ(output.dimensions.size(), 2);
    EXPECT_EQ(output.dimensions[0], 13);
    EXPECT_EQ(output.dimensions[1], 19);
}

TEST_P(UnarySpecialCheckTest, SeedErrors) {
    auto version = GetParam();

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        unary_special_check_opener(fhandle, "hello", "is_nan", { 13, 19 }, version, "STRING");
    }
    expect_error([&]() -> void { chihaya::validate(path, "hello"); }, "integer, float or boolean");

    {
        H5::H5File fhandle(path, H5F_ACC_RDWR);
        auto ghandle = fhandle.openGroup("hello");
        ghandle.unlink("seed");
    }
    expect_error([&]() -> void { chihaya::validate(path, "hello"); }, "expected a group at 'seed'");
}

TEST_P(UnarySpecialCheckTest, MethodErrors) {
    auto version = GetParam();

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = unary_special_check_opener(fhandle, "hello", "is_nan", { 13, 19 }, version, "FLOAT");
        ghandle.unlink("method");
    }
    expect_error([&]() -> void { chihaya::validate(path, "hello"); }, "expected a dataset at 'method'");

    {
        H5::H5File fhandle(path, H5F_ACC_RDWR);
        auto ghandle = fhandle.openGroup("hello");
        add_string_scalar(ghandle, "method", "foo");
    }
    expect_error([&]() -> void { chihaya::validate(path, "hello"); }, "unrecognized 'method'");
}

INSTANTIATE_TEST_SUITE_P(
    UnarySpecialCheck,
    UnarySpecialCheckTest,
    ::testing::Values(0, 1000000, 1100000)
);
