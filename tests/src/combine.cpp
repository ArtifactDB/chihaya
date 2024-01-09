#include <gtest/gtest.h>
#include "chihaya/chihaya.hpp"
#include "utils.h"

TEST(Combine, Simple) {
    std::string path = "Test_combine.h5";

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = operation_opener(fhandle, "hello", "combine");
        add_scalar(ghandle, "along", 0);

        auto lhandle = list_opener(ghandle, "seeds", 2);
        external_array_opener(lhandle, "0", { 13, 19 });
        external_array_opener(lhandle, "1", { 20, 19 }); 
    }

    auto output = chihaya::validate(path, "hello"); 
    EXPECT_EQ(output.type, chihaya::FLOAT);
    const auto& dims = output.dimensions;
    EXPECT_EQ(dims[0], 33);
    EXPECT_EQ(dims[1], 19);
}

TEST(Combine, Mixed) {
    std::string path = "Test_combine.h5";

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = operation_opener(fhandle, "hello", "combine");
        add_scalar(ghandle, "along", 1);

        auto lhandle = list_opener(ghandle, "seeds", 3);
        external_array_opener(lhandle, "0", { 13, 10 }, "BOOLEAN");
        external_array_opener(lhandle, "1", { 13, 20 }, "INTEGER"); 
        external_array_opener(lhandle, "2", { 13, 30 }, "BOOLEAN"); 
    }

    auto output = chihaya::validate(path, "hello"); 
    EXPECT_EQ(output.type, chihaya::INTEGER);
    const auto& dims = output.dimensions;
    EXPECT_EQ(dims[0], 13);
    EXPECT_EQ(dims[1], 60);
}

TEST(Combine, AlongErrors) {
    std::string path = "Test_combine.h5";

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = operation_opener(fhandle, "hello", "combine");
    }
    expect_error([&]() -> void { chihaya::validate(path, "hello"); }, "expected a dataset at 'along'");

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = operation_opener(fhandle, "hello", "combine");
        add_vector<int>(ghandle, "along", { 1 });
    }
    expect_error([&]() -> void { chihaya::validate(path, "hello"); }, "'along' should be a scalar dataset");

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = operation_opener(fhandle, "hello", "combine");
        add_scalar(ghandle, "along", -1);
    }
    expect_error([&]() -> void { chihaya::validate(path, "hello"); }, "'along' should be non-negative");

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = operation_opener(fhandle, "hello", "combine");
        add_scalar(ghandle, "along", 2);

        auto lhandle = list_opener(ghandle, "seeds", 1);
        external_array_opener(lhandle, "0", { 13, 10 }, "BOOLEAN");
    }
    expect_error([&]() -> void { chihaya::validate(path, "hello"); }, "'along' should be less than the seed dimensionality");
}

TEST(Combine, SeedErrors) {
    std::string path = "Test_combine.h5";

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = operation_opener(fhandle, "hello", "combine");
        add_scalar(ghandle, "along", 0);
    }
    expect_error([&]() -> void { chihaya::validate(path, "hello"); }, "expected a group at 'seeds'");

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = operation_opener(fhandle, "hello", "combine");
        add_scalar(ghandle, "along", 0);
        auto lhandle = list_opener(ghandle, "seeds", 1);
        lhandle.removeAttr("delayed_length");
    }
    expect_error([&]() -> void { chihaya::validate(path, "hello"); }, "failed to load 'seeds' list");

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = operation_opener(fhandle, "hello", "combine");
        add_scalar(ghandle, "along", 0);
        list_opener(ghandle, "seeds", 1);
    }
    expect_error([&]() -> void { chihaya::validate(path, "hello"); }, "missing elements in the 'seeds' list");

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = operation_opener(fhandle, "hello", "combine");
        add_scalar(ghandle, "along", 0);

        auto lhandle = list_opener(ghandle, "seeds", 2);
        external_array_opener(lhandle, "0", { 13, 10 }, "BOOLEAN");
        external_array_opener(lhandle, "1", { 13, 10, 5 }, "BOOLEAN");
    }
    expect_error([&]() -> void { chihaya::validate(path, "hello"); }, "dimensionality mismatch");

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = operation_opener(fhandle, "hello", "combine");
        add_scalar(ghandle, "along", 0);

        auto lhandle = list_opener(ghandle, "seeds", 2);
        external_array_opener(lhandle, "0", { 13, 10 }, "BOOLEAN");
        external_array_opener(lhandle, "1", { 5, 15 }, "BOOLEAN");
    }
    expect_error([&]() -> void { chihaya::validate(path, "hello"); }, "inconsistent dimension extents");
}
