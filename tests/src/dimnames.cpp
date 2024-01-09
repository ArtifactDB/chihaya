#include <gtest/gtest.h>
#include "chihaya/chihaya.hpp"
#include "utils.h"

TEST(Dimnames, NoOp) {
    std::string path = "Test_dimnames.h5";

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = operation_opener(fhandle, "hello", "dimnames");
        external_array_opener(ghandle, "seed", { 13, 19 });
        list_opener(ghandle, "dimnames", 2);
    }

    auto output = chihaya::validate(path, "hello"); 
    EXPECT_EQ(output.type, chihaya::FLOAT);
    const auto& dims = output.dimensions;
    EXPECT_EQ(dims[0], 13);
    EXPECT_EQ(dims[1], 19);
}

TEST(Dimnames, Partial) {
    std::string path = "Test_dimnames.h5";

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = operation_opener(fhandle, "hello", "dimnames");
        external_array_opener(ghandle, "seed", { 13, 10 }, "INTEGER");

        auto lhandle = list_opener(ghandle, "dimnames", 2);
        add_string_vector(lhandle, "1", 10);
    }

    auto output = chihaya::validate(path, "hello"); 
    EXPECT_EQ(output.type, chihaya::INTEGER);
    const auto& dims = output.dimensions;
    EXPECT_EQ(dims[0], 13);
    EXPECT_EQ(dims[1], 10);
}

TEST(Dimnames, Full) {
    std::string path = "Test_dimnames.h5";

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = operation_opener(fhandle, "hello", "dimnames");
        external_array_opener(ghandle, "seed", { 13, 10 }, "INTEGER");

        auto lhandle = list_opener(ghandle, "dimnames", 2);
        add_string_vector(lhandle, "0", 13);
        add_string_vector(lhandle, "1", 10);
    }

    auto output = chihaya::validate(path, "hello"); 
    EXPECT_EQ(output.type, chihaya::INTEGER);
    const auto& dims = output.dimensions;
    EXPECT_EQ(dims[0], 13);
    EXPECT_EQ(dims[1], 10);
}

TEST(Dimnames, Errors) {
    std::string path = "Test_dimnames.h5";

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = operation_opener(fhandle, "hello", "dimnames");
    }
    expect_error([&]() -> void { chihaya::validate(path, "hello"); }, "expected a group at 'seed'");

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = operation_opener(fhandle, "hello", "dimnames");
        external_array_opener(ghandle, "seed", { 5, 7 });
    }
    expect_error([&]() -> void { chihaya::validate(path, "hello"); }, "expected a 'dimnames' group");

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = operation_opener(fhandle, "hello", "dimnames");
        external_array_opener(ghandle, "seed", { 5, 7 });
        auto lhandle = list_opener(ghandle, "dimnames", 2);
        lhandle.removeAttr("delayed_length");
    }
    expect_error([&]() -> void { chihaya::validate(path, "hello"); }, "failed to load 'dimnames' list");

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = operation_opener(fhandle, "hello", "dimnames");
        external_array_opener(ghandle, "seed", { 5, 7 });
        auto lhandle = list_opener(ghandle, "dimnames", 3);
    }
    expect_error([&]() -> void { chihaya::validate(path, "hello"); }, "length of 'dimnames' list");

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = operation_opener(fhandle, "hello", "dimnames");
        external_array_opener(ghandle, "seed", { 5, 7 });
        auto lhandle = list_opener(ghandle, "dimnames", 2);
        lhandle.createGroup("0");
    }
    expect_error([&]() -> void { chihaya::validate(path, "hello"); }, "expected a dataset at '0'");

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = operation_opener(fhandle, "hello", "dimnames");
        external_array_opener(ghandle, "seed", { 5, 7 });
        auto lhandle = list_opener(ghandle, "dimnames", 2);
        add_vector<int>(lhandle, "0", {1});
    }
    expect_error([&]() -> void { chihaya::validate(path, "hello"); }, "should be a 1-dimensional string dataset");

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = operation_opener(fhandle, "hello", "dimnames");
        external_array_opener(ghandle, "seed", { 5, 7 });
        auto lhandle = list_opener(ghandle, "dimnames", 2);
        add_string_vector(lhandle, "0", 1);
    }
    expect_error([&]() -> void { chihaya::validate(path, "hello"); }, "length equal to the extent");
}
