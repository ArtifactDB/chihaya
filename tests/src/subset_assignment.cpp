#include <gtest/gtest.h>
#include "chihaya/chihaya.hpp"
#include "utils.h"

TEST(SubsetAssignment, NoOp) {
    std::string path = "Test_subset_assignment.h5";

    // Mocking up a file.
    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = operation_opener(fhandle, "hello", "subset assignment");
        external_array_opener(ghandle, "seed", { 13, 19 }); 
        external_array_opener(ghandle, "value", { 13, 19 }, "STRING"); 
        auto lhandle = list_opener(ghandle, "index", 2);
    }

    auto output = chihaya::validate(path, "hello"); 
    EXPECT_EQ(output.type, chihaya::STRING);
    const auto& dims = output.dimensions;
    EXPECT_EQ(dims[0], 13);
    EXPECT_EQ(dims[1], 19);
}

TEST(SubsetAssignment, AllSubsets) {
    std::string path = "Test_subset_assignment.h5";

    // Mocking up a file.
    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = operation_opener(fhandle, "hello", "subset assignment");
        external_array_opener(ghandle, "seed", { 13, 19 }, "INTEGER"); 
        external_array_opener(ghandle, "value", { 6, 7 }); 

        auto lhandle = list_opener(ghandle, "index", 2);
        add_vector<int>(lhandle, "0", { 1, 3, 0, 2, 9, 12 });
        add_vector<int>(lhandle, "1", { 2, 2, 5, 7, 9, 9, 12 });
    }

    auto output = chihaya::validate(path, "hello"); 
    EXPECT_EQ(output.type, chihaya::FLOAT);
    const auto& dims = output.dimensions;
    EXPECT_EQ(dims[0], 13);
    EXPECT_EQ(dims[1], 19);
}

TEST(SubsetAssignment, OneSubset) {
    std::string path = "Test_subset_assignment.h5";

    // Mocking up a file.
    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = operation_opener(fhandle, "hello", "subset assignment");
        external_array_opener(ghandle, "seed", { 13, 19 });
        external_array_opener(ghandle, "value", { 13, 7 });

        auto lhandle = list_opener(ghandle, "index", 2);
        add_vector<int>(lhandle, "1", { 2, 2, 5, 7, 9, 9, 12 });
    }

    auto output = chihaya::validate(path, "hello"); 
    const auto& dims = output.dimensions;
    EXPECT_EQ(dims[0], 13);
    EXPECT_EQ(dims[1], 19);
}

TEST(SubsetAssignment, Errors) {
    std::string path = "Test_subset_assignment.h5";

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        operation_opener(fhandle, "hello", "subset assignment");
    }
    expect_error([&]() -> void { chihaya::validate(path, "hello"); }, "expected a group at 'seed'");

    {
        H5::H5File fhandle(path, H5F_ACC_RDWR);
        auto ghandle = fhandle.openGroup("hello");
        external_array_opener(ghandle, "seed", { 13, 19 }); 
    }
    expect_error([&]() -> void { chihaya::validate(path, "hello"); }, "expected a group at 'value'");

    {
        H5::H5File fhandle(path, H5F_ACC_RDWR);
        auto ghandle = fhandle.openGroup("hello");
        external_array_opener(ghandle, "value", { 6, 7 }); 
    }
    expect_error([&]() -> void { chihaya::validate(path, "hello"); }, "expected a group at 'index'");

    {
        H5::H5File fhandle(path, H5F_ACC_RDWR);
        auto ghandle = fhandle.openGroup("hello");
        auto lhandle = list_opener(ghandle, "index", 2);
        add_vector<int>(lhandle, "2", { 1, 3, 0, 2, 9 });
    }
    expect_error([&]() -> void { chihaya::validate(path, "hello"); }, "failed to load 'index'");

    {
        H5::H5File fhandle(path, H5F_ACC_RDWR);
        auto ghandle = fhandle.openGroup("hello");
        auto lhandle = ghandle.openGroup("index");
        lhandle.unlink("2"); // removing the above.
        lhandle.createGroup("0");
    }
    expect_error([&]() -> void { chihaya::validate(path, "hello"); }, "expected a dataset");

    {
        H5::H5File fhandle(path, H5F_ACC_RDWR);
        auto ghandle = fhandle.openGroup("hello");
        auto lhandle = ghandle.openGroup("index");
        lhandle.unlink("0"); // removing the above.
        add_vector<double>(lhandle, "0", { 1, 3, 0, 2, 9 });
    }
    expect_error([&]() -> void { chihaya::validate(path, "hello"); }, "expected an integer dataset");

    {
        H5::H5File fhandle(path, H5F_ACC_RDWR);
        auto ghandle = fhandle.openGroup("hello");
        auto lhandle = ghandle.openGroup("index");
        lhandle.unlink("0"); // removing the above.
        add_vector<int>(lhandle, "0", { 1, 3, 0, 2, 1, 9, 5 });
    }
    expect_error([&]() -> void { chihaya::validate(path, "hello"); }, "dimension extents are not consistent");

    {
        H5::H5File fhandle(path, H5F_ACC_RDWR);
        auto ghandle = fhandle.openGroup("hello");
        auto lhandle = ghandle.openGroup("index");
        lhandle.unlink("0"); // removing the above.
        add_vector<int>(lhandle, "0", { 1, 3, 0, 2, 9, 100 });
    }
    expect_error([&]() -> void { chihaya::validate(path, "hello"); }, "out of range");
}
