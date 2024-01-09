#include <gtest/gtest.h>
#include "chihaya/chihaya.hpp"
#include "utils.h"

inline H5::Group external_array_opener(const H5::Group& parent, const std::string& name, const std::vector<int>& dimensions, std::string type = "FLOAT") {
    auto ghandle = custom_array_opener(parent, name, dimensions, type, "external hdf5 dense array");

    H5::StrType stype(0, H5T_VARIABLE);
    auto fhandle = ghandle.createDataSet("file", stype, H5S_SCALAR);
    std::string dummy = "WHEEE";
    fhandle.write(dummy, stype, H5S_SCALAR);

    auto nhandle = ghandle.createDataSet("name", stype, H5S_SCALAR);
    nhandle.write(dummy, stype, H5S_SCALAR);

    return ghandle;
}


TEST(ExternalHDF5, Basic) {
    std::string path = "Test_external.h5";

    // Re-run of the same tests in custom_array.cpp,
    // so we won't go through that whole thing again.
    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        external_array_opener(fhandle, "ext", { 50, 5, 10 }); 
    }
    {
        auto output = chihaya::validate(path, "ext"); 
        EXPECT_EQ(output.type, chihaya::FLOAT);

        const auto& dims = output.dimensions;
        EXPECT_EQ(dims.size(), 3);
        EXPECT_EQ(dims[0], 50);
        EXPECT_EQ(dims[1], 5);
        EXPECT_EQ(dims[2], 10);
    }
}

TEST(ExternalHDF5, Errors) {
    std::string path = "Test_external.h5";

    /*** Skipping the checks that are shared with custom_array.cpp. ***/

    // Checking for file.
    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = external_array_opener(fhandle, "ext", { 50, 5, 10 }); 
        ghandle.unlink("file");
    }
    expect_error([&]() -> void { chihaya::validate(path, "ext"); }, "expected a dataset at 'file'");

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = external_array_opener(fhandle, "ext", { 50, 5, 10 }); 
        ghandle.unlink("file");

        hsize_t dims = 10;
        H5::DataSpace dspace(1, &dims);
        ghandle.createDataSet("file", H5::PredType::NATIVE_INT, dspace);
    }
    expect_error([&]() -> void { chihaya::validate(path, "ext"); }, "'file' should be a scalar");

    // Checking for name.
    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = external_array_opener(fhandle, "ext", { 50, 5, 10 }); 
        ghandle.unlink("name");
    }
    expect_error([&]() -> void { chihaya::validate(path, "ext"); }, "expected a dataset at 'name'");

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = external_array_opener(fhandle, "ext", { 50, 5, 10 }); 
        ghandle.unlink("name");

        hsize_t dims = 10;
        H5::DataSpace dspace(1, &dims);
        ghandle.createDataSet("name", H5::PredType::NATIVE_INT, dspace);
    }
    expect_error([&]() -> void { chihaya::validate(path, "ext"); }, "'name' should be a scalar");
}
