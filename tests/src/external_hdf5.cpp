#include <gtest/gtest.h>

#include <string>
#include <vector>
#include <cstddef>

#include "chihaya/external_hdf5.hpp"

#include "utils.h"

static H5::Group external_array_opener(
    const H5::Group& handle,
    const std::string& name,
    const std::vector<std::size_t>& dimensions,
    const ritsuko::Version& version,
    const std::string& type
) {
    auto ghandle = array_opener(handle, name, "external hdf5 thingy");
    add_version_string(ghandle, version);
    add_string_scalar(ghandle, "type", type);
    add_numeric_vector(ghandle, "dimensions", dimensions, H5::PredType::NATIVE_INT);
    add_string_scalar(ghandle, "file", "FOO");
    add_string_scalar(ghandle, "name", "BAR");
    return ghandle;
}

/***********************************/

class ExternalHdf5PassTest : public ::testing::TestWithParam<std::tuple<ritsuko::Version, bool> > {};

TEST_P(ExternalHdf5PassTest, Basic) {
    const auto path = define_test_path("external_array");
    auto params = GetParam();
    auto version = std::get<0>(params);
    auto deets = std::get<1>(params);

    std::vector<std::size_t> dimensions{ 50, 5, 10 };
    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        external_array_opener(fhandle, "ext", dimensions, version, "FLOAT"); 
    }

    auto output = test_validate(path, "ext", deets); 
    EXPECT_EQ(output.type, chihaya::FLOAT);
    EXPECT_EQ(output.dimensions, dimensions);
}

INSTANTIATE_TEST_SUITE_P(
    ExternalHdf5,
    ExternalHdf5PassTest,
    ::testing::Combine(
        ::testing::Values(ritsuko::Version(0, 99, 0), ritsuko::Version(1, 0, 0)), // this mode is deprecated in >=1.1
        ::testing::Values(false, true)
    )
);

/***********************************/

class ExternalHdf5ErrorTest : public ::testing::TestWithParam<ritsuko::Version> {};

TEST_P(ExternalHdf5ErrorTest, File) {
    const auto path = define_test_path("external_array");
    auto version = GetParam();

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = external_array_opener(fhandle, "ext", { 50, 5, 10 }, version, "FLOAT"); 
        ghandle.unlink("file");
        add_string_vector(ghandle, "file", 5, /* strlen = */ 2);
    }

    expect_error(path, "ext", "should be scalar");
}

TEST_P(ExternalHdf5ErrorTest, Name) {
    const auto path = define_test_path("external_array");
    auto version = GetParam();

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = external_array_opener(fhandle, "ext", { 50, 5, 10 }, version, "FLOAT"); 
        ghandle.unlink("name");
        add_string_vector(ghandle, "name", 5, /* strlen = */ 2);
    }

    expect_error(path, "ext", "should be scalar");
}

INSTANTIATE_TEST_SUITE_P(
    ExternalHdf5,
    ExternalHdf5ErrorTest,
    ::testing::Values(ritsuko::Version(0, 99, 0), ritsuko::Version(1, 0, 0)) // this mode is deprecated in >=1.1
);

/***********************************/

TEST(ExternalHdf5, Latest) {
    const auto path = define_test_path("external_array");
    ritsuko::Version version(1, 1, 0);

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = external_array_opener(fhandle, "ext", { 50, 5, 10 }, version, "FLOAT"); 
    }

    H5::H5File fhandle(path, H5F_ACC_RDONLY);
    chihaya::Options options;
    expect_error(
        [&]() -> void {
            chihaya::validate_external_hdf5(fhandle.openGroup("ext"), version, options);
        },
        "deprecated"
    );
}
