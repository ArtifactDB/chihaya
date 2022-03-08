#include <gtest/gtest.h>
#include "chihaya/chihaya.hpp"
#include "utils.h"

TEST(External, Basic) {
    std::string path = "test_external.h5";

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        external_array_opener(fhandle, "ext", { 50, 5, 10 }); 
    }

    auto output = chihaya::validate(path, "ext"); 
    EXPECT_EQ(output.size(), 3);
    EXPECT_EQ(output[0], 50);
    EXPECT_EQ(output[1], 5);
    EXPECT_EQ(output[2], 10);
}

TEST(External, Errors) {
    std::string path = "test_external.h5";

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = external_array_opener(fhandle, "ext", { 50, 5, 10 }); 
        ghandle.unlink("dimensions");
    }
    expect_error([&]() -> void { chihaya::validate(path, "ext"); }, "expected 'dimensions'");

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        external_array_opener(fhandle, "ext", { 50, -20 }); 
    }
    expect_error([&]() -> void { chihaya::validate(path, "ext"); }, "non-negative");

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = external_array_opener(fhandle, "ext", { 50, -20 }); 
        ghandle.unlink("dimensions");

        hsize_t dims[2];
        dims[0] = 10;
        dims[1] = 10;
        H5::DataSpace dspace(2, dims);
        ghandle.createDataSet("dimensions", H5::PredType::NATIVE_INT, dspace);
    }
    expect_error([&]() -> void { chihaya::validate(path, "ext"); }, "1-dimensional");
}

