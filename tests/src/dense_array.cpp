#include <gtest/gtest.h>
#include "chihaya/chihaya.hpp"
#include "utils.h"

template<class T>
H5::Group dense_array_opener(H5::Group& handle, std::string name, std::vector<hsize_t> dimensions, const T& type, bool native = true) {
    auto ghandle = array_opener(handle, name, "dense array");

    if (!native) {
        std::reverse(dimensions.begin(), dimensions.end());
    }
    H5::DataSpace dspace(dimensions.size(), dimensions.data());
    ghandle.createDataSet("data", type, dspace);

    auto dhandle = ghandle.createDataSet("native", H5::PredType::NATIVE_UINT8, H5S_SCALAR);
    int native_int = native;
    dhandle.write(&native_int, H5::PredType::NATIVE_INT);

    return ghandle;
}

TEST(Dense, Basic) {
    std::string path = "Test_dense_array.h5";

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        dense_array_opener(fhandle, "dense", { 20, 17 }, H5::PredType::NATIVE_INT); 
    }
    {
        auto output = chihaya::validate(path, "dense"); 
        EXPECT_EQ(output.type, chihaya::INTEGER);
        const auto& dims = output.dimensions;
        EXPECT_EQ(dims.size(), 2);
        EXPECT_EQ(dims[0], 20);
        EXPECT_EQ(dims[1], 17);
    }

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        dense_array_opener(fhandle, "dense", { 5, 17 }, H5::PredType::NATIVE_FLOAT); 
    }
    {
        auto output = chihaya::validate(path, "dense"); 
        EXPECT_EQ(output.type, chihaya::FLOAT);
        const auto& dims = output.dimensions;
        EXPECT_EQ(dims.size(), 2);
        EXPECT_EQ(dims[0], 5);
        EXPECT_EQ(dims[1], 17);
    }
}

TEST(Dense, NonNative) {
    std::string path = "Test_dense_array.h5";

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        dense_array_opener(fhandle, "dense", { 20, 17 }, H5::PredType::NATIVE_INT, false); 
    }
    {
        auto output = chihaya::validate(path, "dense"); 
        EXPECT_EQ(output.type, chihaya::INTEGER);
        const auto& dims = output.dimensions;
        EXPECT_EQ(dims.size(), 2);
        EXPECT_EQ(dims[0], 20);
        EXPECT_EQ(dims[1], 17);
    }
}

TEST(Dense, Missing) {
    std::string path = "Test_dense_array.h5";

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        dense_array_opener(fhandle, "dense", { 20, 17 }, H5::PredType::NATIVE_INT, false); 
        auto dhandle = fhandle.openDataSet("dense/data");
        add_missing_placeholder(dhandle, (int)2);
    }
    {
        auto output = chihaya::validate(path, "dense"); 
        EXPECT_EQ(output.type, chihaya::INTEGER);
    }
}


TEST(Dense, Dimnames) {
    std::string path = "Test_dense_array.h5";

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = dense_array_opener(fhandle, "dense", { 20, 17 }, H5::PredType::NATIVE_INT); 
        auto lhandle = list_opener(ghandle, "dimnames", 2);
        add_string_vector(lhandle, "0", 20);
        add_string_vector(lhandle, "1", 17);
    }
    {
        auto output = chihaya::validate(path, "dense"); 
        EXPECT_EQ(output.type, chihaya::INTEGER);
    }
}

TEST(Dense, Boolean) {
    std::string path = "Test_dense_array.h5";

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = dense_array_opener(fhandle, "dense", { 20, 17 }, H5::PredType::NATIVE_INT); 
        auto dhandle = ghandle.openDataSet("data");
        auto ahandle = dhandle.createAttribute("is_boolean", H5::PredType::NATIVE_INT, H5S_SCALAR);
        int val = 1;
        ahandle.write(H5::PredType::NATIVE_INT, &val);
    }
    {
        auto output = chihaya::validate(path, "dense"); 
        EXPECT_EQ(output.type, chihaya::BOOLEAN);
    }
}

TEST(Dense, Errors) {
    std::string path = "Test_dense_array.h5";

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = dense_array_opener(fhandle, "dense", { 20, 17 }, H5::PredType::NATIVE_FLOAT);
        ghandle.unlink("data");
    }
    expect_error([&]() -> void { chihaya::validate(path, "dense"); }, "'data' should be a dataset");

    {
        H5::H5File fhandle(path, H5F_ACC_RDWR);
        auto ghandle = fhandle.openGroup("dense");
        add_scalar<int>(ghandle, "data", 50);
    }
    expect_error([&]() -> void { chihaya::validate(path, "dense"); }, "'data' should have non-zero");

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = dense_array_opener(fhandle, "dense", { 50, 10 }, H5::PredType::NATIVE_INT); 
        auto lhandle = list_opener(ghandle, "dimnames", 2);
        lhandle.createGroup("0");
    }
    expect_error([&]() -> void { chihaya::validate(path, "dense"); }, "dimnames");

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = dense_array_opener(fhandle, "dense", { 50, 10 }, H5::PredType::NATIVE_FLOAT); 
        auto dhandle = ghandle.openDataSet("data");
        dhandle.createAttribute("is_boolean", H5::PredType::NATIVE_INT, H5S_SCALAR);
    }
    expect_error([&]() -> void { chihaya::validate(path, "dense"); }, "should only exist for integer");

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = dense_array_opener(fhandle, "dense", { 50, 10 }, H5::PredType::NATIVE_INT); 
        auto dhandle = ghandle.openDataSet("data");
        H5::StrType stype(0, H5T_VARIABLE);
        dhandle.createAttribute("is_boolean", stype, H5S_SCALAR);
    }
    expect_error([&]() -> void { chihaya::validate(path, "dense"); }, "should be an integer scalar");

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        dense_array_opener(fhandle, "dense", { 20, 17 }, H5::PredType::NATIVE_INT); 
        auto dhandle = fhandle.openDataSet("dense/data");
        add_missing_placeholder(dhandle, 2.5);
    }
    expect_error([&]() -> void { chihaya::validate(path, "dense"); }, "should be of the same type");
}
