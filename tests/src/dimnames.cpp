#include <gtest/gtest.h>

#include <cstddef>
#include <vector>
#include <string>

#include "H5Cpp.h"
#include "ritsuko/ritsuko.hpp"
#include "chihaya/dimnames.hpp"

#include "utils.h"

static H5::Group dimnames_opener(H5::Group& handle, const std::string& name, const std::vector<std::size_t>& dims, const std::string& type, const ritsuko::Version& version) {
    auto ghandle = operation_opener(handle, name, "dimnames");
    add_version_string(ghandle, version);
    mock_array_opener(ghandle, "seed", dims, version, type);
    list_opener(ghandle, "dimnames", dims.size(), version);
    return ghandle;
}

/***********************************/

class DimnamesPassTest : public ::testing::TestWithParam<std::tuple<ritsuko::Version, bool> > {};

TEST_P(DimnamesPassTest, Basic) {
    auto path = define_test_path("dimnames");
    auto params = GetParam();
    auto version = std::get<0>(params);
    auto deets = std::get<1>(params);

    std::vector<std::size_t> dimensions{ 12, 20 };
    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = dimnames_opener(fhandle, "hello", dimensions, "INTEGER", version);
        auto lhandle = ghandle.openGroup("dimnames");
        add_string_vector(lhandle, "0", 12, /* len = */ 5);
        add_string_vector(lhandle, "1", 20, /* len = */ 2);
    }

    auto output = test_validate(path, "hello", deets); 
    EXPECT_EQ(output.type, chihaya::INTEGER);
    EXPECT_EQ(output.dimensions, dimensions);
}

INSTANTIATE_TEST_SUITE_P(
    Dimnames,
    DimnamesPassTest,
    ::testing::Combine(
        spawn_all_versions(),
        ::testing::Values(false, true)
    )
);

/***********************************/

class DimnamesErrorTest : public ::testing::TestWithParam<ritsuko::Version> {};

TEST_P(DimnamesErrorTest, Dimnames) {
    auto path = define_test_path("dimnames");
    auto version = GetParam();

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = dimnames_opener(fhandle, "hello", { 0, 0 }, "INTEGER", version);
        ghandle.unlink("dimnames");
        list_opener(ghandle, "dimnames", 3, version);
    }
    expect_error(path, "hello", "length of 'dimnames' list");
}

INSTANTIATE_TEST_SUITE_P(
    Dimnames,
    DimnamesErrorTest,
    spawn_all_versions()
);
