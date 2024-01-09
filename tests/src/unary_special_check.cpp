#include <gtest/gtest.h>
#include "chihaya/chihaya.hpp"
#include "utils.h"

TEST(UnarySpecialCheck, Basic) {
    std::string path = "Test_unary_special_check.h5";

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = operation_opener(fhandle, "hello", "unary special check");
        external_array_opener(ghandle, "seed", { 13, 19 }, "INTEGER"); 
        add_scalar(ghandle, "method", std::string("is_nan"));
    }

    auto output = chihaya::validate(path, "hello"); 
    EXPECT_EQ(output.type, chihaya::BOOLEAN);
}

TEST(UnarySpecialCheck, Errors) {
    std::string path = "Test_unary_special_check.h5";

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        operation_opener(fhandle, "hello", "unary special check");
    }
    expect_error([&]() -> void { chihaya::validate(path, "hello"); }, "expected a group at 'seed'");

    {
        H5::H5File fhandle(path, H5F_ACC_RDWR);
        auto ghandle = fhandle.openGroup("hello");
        external_array_opener(ghandle, "seed", { 13, 19 }, "STRING"); 
    }
    expect_error([&]() -> void { chihaya::validate(path, "hello"); }, "should contain numeric or boolean");

    {
        H5::H5File fhandle(path, H5F_ACC_RDWR);
        auto ghandle = fhandle.openGroup("hello");
        ghandle.unlink("seed");
        external_array_opener(ghandle, "seed", { 13, 19 }, "INTEGER"); 
    }
    expect_error([&]() -> void { chihaya::validate(path, "hello"); }, "expected a dataset at 'method'");

    {
        H5::H5File fhandle(path, H5F_ACC_RDWR);
        auto ghandle = fhandle.openGroup("hello");
        add_scalar(ghandle, "method", std::string("foo"));
    }
    expect_error([&]() -> void { chihaya::validate(path, "hello"); }, "unrecognized 'method'");
}
