#include <gtest/gtest.h>
#include "chihaya/chihaya.hpp"
#include "utils.h"

class ExternalHdf5Test : public ::testing::TestWithParam<int> {
public:
    ExternalHdf5Test() : path("Test_external.h5") {}

protected:
    std::string path;

    static H5::Group external_array_opener(const H5::Group& handle, const std::string& name, const std::vector<int>& dimensions, int version, std::string type) {
        auto ghandle = array_opener(handle, name, "external hdf5 thingy");
        add_version_string(ghandle, version);
        add_string_scalar(ghandle, "type", type);

        if (version < 1100000) {
            add_numeric_vector(ghandle, "dimensions", dimensions, H5::PredType::NATIVE_INT);
        } else {
            add_numeric_vector(ghandle, "dimensions", dimensions, H5::PredType::NATIVE_UINT32);
        }

        add_string_scalar(ghandle, "file", "WHEE");
        add_string_scalar(ghandle, "name", "WHEE");
        return ghandle;
    }
};

TEST_P(ExternalHdf5Test, Basic) {
    auto version = GetParam();
    if (version >= 1100000) {
        return;
    }

    // Re-run of the same tests in custom_array.cpp,
    // so we won't go through that whole thing again.
    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        external_array_opener(fhandle, "ext", { 50, 5, 10 }, version, "FLOAT"); 
    }
    {
        auto output = test_validate(path, "ext"); 
        EXPECT_EQ(output.type, chihaya::FLOAT);

        const auto& dims = output.dimensions;
        EXPECT_EQ(dims.size(), 3);
        EXPECT_EQ(dims[0], 50);
        EXPECT_EQ(dims[1], 5);
        EXPECT_EQ(dims[2], 10);
    }
}

TEST_P(ExternalHdf5Test, Errors) {
    auto version = GetParam();
    if (version >= 1100000) {
        // Deprecated...
        {
            H5::H5File fhandle(path, H5F_ACC_TRUNC);
            external_array_opener(fhandle, "ext", { 50, 5, 10 }, version, "FLOAT"); 
        }
        expect_error(path, "ext", "unknown array type");
        return;
    }

    /*** Skipping the checks that are shared with custom_array.cpp. ***/

    // Checking for file.
    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = external_array_opener(fhandle, "ext", { 50, 5, 10 }, version, "FLOAT"); 
        ghandle.unlink("file");
    }
    expect_error(path, "ext", "expected a dataset at 'file'");

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = external_array_opener(fhandle, "ext", { 50, 5, 10 }, version, "FLOAT"); 
        ghandle.unlink("file");
        add_string_vector(ghandle, "file", 5);
    }
    expect_error(path, "ext", "'file' should be a scalar");

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = external_array_opener(fhandle, "ext", { 50, 5, 10 }, version, "FLOAT"); 
        ghandle.unlink("file");
        add_numeric_scalar<int>(ghandle, "file", 5, H5::PredType::NATIVE_INT);
    }
    expect_error(path, "ext", "string");

    // Checking for name.
    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = external_array_opener(fhandle, "ext", { 50, 5, 10 }, version, "FLOAT"); 
        ghandle.unlink("name");
    }
    expect_error(path, "ext", "expected a dataset at 'name'");

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = external_array_opener(fhandle, "ext", { 50, 5, 10 }, version, "FLOAT"); 
        ghandle.unlink("name");
        add_string_vector(ghandle, "name", 5);
    }
    expect_error(path, "ext", "'name' should be a scalar");

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = external_array_opener(fhandle, "ext", { 50, 5, 10 }, version, "FLOAT"); 
        ghandle.unlink("name");
        add_numeric_scalar<int>(ghandle, "name", 5, H5::PredType::NATIVE_INT);
    }
    expect_error(path, "ext", "string");
}

INSTANTIATE_TEST_SUITE_P(
    ExternalHdf5,
    ExternalHdf5Test,
    ::testing::Values(0, 1000000, 1100000)
);
