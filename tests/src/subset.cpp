#include <gtest/gtest.h>
#include "chihaya/chihaya.hpp"
#include "utils.h"

TEST(Subset, NoOp) {
    std::string path = "test_subset.h5";

    // Mocking up a file.
    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = operation_opener(fhandle, "hello", "subset");
        external_array_opener(ghandle, "seed", { 13, 19 }); 
        auto lhandle = list_opener(ghandle, "index", 2);
    }

    auto output = chihaya::validate(path, "hello"); 
    EXPECT_EQ(output[0], 13);
    EXPECT_EQ(output[1], 19);
}

TEST(Subset, AllSubsets) {
    std::string path = "temp_subset.h5";

    // Mocking up a file.
    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = operation_opener(fhandle, "hello", "subset");
        external_array_opener(ghandle, "seed", { 13, 19 }); 
        auto lhandle = list_opener(ghandle, "index", 2);
        add_vector<int>(lhandle, "0", { 1, 3, 0, 2, 9, 12 });
        add_vector<int>(lhandle, "1", { 2, 2, 5, 7, 9, 9, 12 });
    }

    auto output = chihaya::validate(path, "hello"); 
    EXPECT_EQ(output[0], 6);
    EXPECT_EQ(output[1], 7);
}

TEST(Subset, OneSubset) {
    std::string path = "temp_subset.h5";

    // Mocking up a file.
    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = operation_opener(fhandle, "hello", "subset");
        external_array_opener(ghandle, "seed", { 13, 19 }); 
        auto lhandle = list_opener(ghandle, "index", 2);
        add_vector<int>(lhandle, "1", { 2, 2, 5, 7, 9, 9, 12 });
    }

    auto output = chihaya::validate(path, "hello"); 
    EXPECT_EQ(output[0], 13);
    EXPECT_EQ(output[1], 7);
}

TEST(Subset, Errors) {
    std::string path = "temp_subset.h5";

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        operation_opener(fhandle, "hello", "subset");
    }
    expect_error([&]() -> void { chihaya::validate(path, "hello"); }, "expected 'seed'");

    {
        H5::H5File fhandle(path, H5F_ACC_RDWR);
        auto ghandle = fhandle.openGroup("hello");
        external_array_opener(ghandle, "seed", { 13, 19 }); 
    }
    expect_error([&]() -> void { chihaya::validate(path, "hello"); }, "expected 'index'");

    {
        H5::H5File fhandle(path, H5F_ACC_RDWR);
        auto ghandle = fhandle.openGroup("hello");
        auto lhandle = list_opener(ghandle, "index", 2);
        add_vector<int>(lhandle, "2", { 1, 3, 0, 2, 9, 100 });
    }
    expect_error([&]() -> void { chihaya::validate(path, "hello"); }, "failed to load 'index'");

    {
        H5::H5File fhandle(path, H5F_ACC_RDWR);
        auto ghandle = fhandle.openGroup("hello");
        auto lhandle = ghandle.openGroup("index");
        lhandle.unlink("2"); // removing the above.
        add_vector<int>(lhandle, "0", { 1, 3, 0, 2, 9, 100 });
    }
    expect_error([&]() -> void { chihaya::validate(path, "hello"); }, "out of range for element '0'");
}
