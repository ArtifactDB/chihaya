#include <gtest/gtest.h>

#include <string>

#include "H5Cpp.h"
#include "ritsuko/ritsuko.hpp"
#include "chihaya/combine.hpp"

#include "utils.h"

static H5::Group combine_opener(H5::Group& handle, const std::string& name, int along, const ritsuko::Version& version) {
    auto ghandle = operation_opener(handle, name, "combine");
    add_version_string(ghandle, version);
    if (version.lt(1, 1, 0)) {
        add_numeric_scalar(ghandle, "along", along, H5::PredType::NATIVE_INT);
    } else {
        add_numeric_scalar(ghandle, "along", along, H5::PredType::NATIVE_UINT32);
    }
    return ghandle;
}

/***********************************/

class CombinePassTest : public ::testing::TestWithParam<std::tuple<ritsuko::Version, bool> > {};

TEST_P(CombinePassTest, Simple) {
    auto path = define_test_path("combine");
    auto params = GetParam();
    auto version = std::get<0>(params);
    auto deets = std::get<1>(params);

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = combine_opener(fhandle, "hello", 0, version);
        auto lhandle = list_opener(ghandle, "seeds", 2, version);
        mock_array_opener<int>(lhandle, "0", { 13, 19 }, version, "FLOAT");
        mock_array_opener<int>(lhandle, "1", { 20, 19 }, version, "FLOAT"); 
    }
    {
        auto output = test_validate(path, "hello", deets);
        EXPECT_EQ(output.type, chihaya::FLOAT);
        EXPECT_EQ(output.dimensions.size(), 2);
        EXPECT_EQ(output.dimensions[0], 33);
        EXPECT_EQ(output.dimensions[1], 19);
    }

    // Works with all-string seeds.
    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = combine_opener(fhandle, "hello", 1, version);
        auto lhandle = list_opener(ghandle, "seeds", 2, version);
        mock_array_opener<int>(lhandle, "0", { 10, 52 }, version, "STRING");
        mock_array_opener<int>(lhandle, "1", { 10, 12 }, version, "STRING"); 
    }
    {
        auto output = test_validate(path, "hello", deets);
        EXPECT_EQ(output.type, chihaya::STRING);
        EXPECT_EQ(output.dimensions.size(), 2);
        EXPECT_EQ(output.dimensions[0], 10);
        EXPECT_EQ(output.dimensions[1], 64);
    }
}

TEST_P(CombinePassTest, MixedType) {
    auto path = define_test_path("combine");
    auto params = GetParam();
    auto version = std::get<0>(params);
    auto deets = std::get<1>(params);

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = combine_opener(fhandle, "hello", 1, version);
        auto lhandle = list_opener(ghandle, "seeds", 3, version);
        mock_array_opener<int>(lhandle, "0", { 13, 10, 5 }, version, "BOOLEAN");
        mock_array_opener<int>(lhandle, "1", { 13, 20, 5 }, version, "INTEGER"); 
        mock_array_opener<int>(lhandle, "2", { 13, 30, 5 }, version, "BOOLEAN"); 
    }
    {
        auto output = test_validate(path, "hello", deets); 
        EXPECT_EQ(output.type, chihaya::INTEGER);
        EXPECT_EQ(output.dimensions.size(), 3);
        EXPECT_EQ(output.dimensions[0], 13);
        EXPECT_EQ(output.dimensions[1], 60);
        EXPECT_EQ(output.dimensions[2], 5);
    }

    // Also involving FLOATs.
    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = combine_opener(fhandle, "hello", 2, version);
        auto lhandle = list_opener(ghandle, "seeds", 4, version);
        mock_array_opener<int>(lhandle, "0", { 10, 7, 20 }, version, "BOOLEAN");
        mock_array_opener<int>(lhandle, "1", { 10, 7, 10 }, version, "INTEGER"); 
        mock_array_opener<int>(lhandle, "2", { 10, 7, 15 }, version, "FLOAT"); 
        mock_array_opener<int>(lhandle, "3", { 10, 7, 10 }, version, "INTEGER"); 
    }
    {
        auto output = test_validate(path, "hello", deets); 
        EXPECT_EQ(output.type, chihaya::FLOAT);
        EXPECT_EQ(output.dimensions.size(), 3);
        EXPECT_EQ(output.dimensions[0], 10);
        EXPECT_EQ(output.dimensions[1], 7);
        EXPECT_EQ(output.dimensions[2], 55);
    }
}

TEST_P(CombinePassTest, Empty) {
    auto path = define_test_path("combine");
    auto params = GetParam();
    auto version = std::get<0>(params);
    auto deets = std::get<1>(params);

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = combine_opener(fhandle, "hello", 99, version);
        auto lhandle = list_opener(ghandle, "seeds", 0, version);
    }
    auto output = test_validate(path, "hello", deets); 
    EXPECT_EQ(output.type, chihaya::BOOLEAN);
    EXPECT_EQ(output.dimensions.size(), 0);
}

INSTANTIATE_TEST_SUITE_P(
    Combine,
    CombinePassTest,
    ::testing::Combine(
        spawn_all_versions(),
        ::testing::Values(false, true)
    )
);

/***********************************/

class CombineErrorTest : public ::testing::TestWithParam<ritsuko::Version> {};

TEST_P(CombineErrorTest, Along) {
    auto path = define_test_path("combine");
    auto version = GetParam();

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = combine_opener(fhandle, "hello", 0, version);
        ghandle.unlink("along");
        add_numeric_vector<int>(ghandle, "along", { 1 }, H5::PredType::NATIVE_INT);
    }
    expect_error(path, "hello", "should be scalar");

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = combine_opener(fhandle, "hello", 2, version);
        auto lhandle = list_opener(ghandle, "seeds", 1, version);
        mock_array_opener<int>(lhandle, "0", { 13, 10 }, version, "BOOLEAN");
    }
    expect_error(path, "hello", "'along' should be less than the seed dimensionality");
}

TEST_P(CombineErrorTest, Seed) {
    auto path = define_test_path("combine");
    auto version = GetParam();

    // Check that the list validation function is run.
    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = combine_opener(fhandle, "hello", 0, version);
        auto lhandle = list_opener(ghandle, "seeds", 1, version);
        if (version.lt(1, 1, 0)) {
            lhandle.removeAttr("delayed_length");
            add_string_attribute(lhandle, "delayed_length", "FOO");
        } else {
            lhandle.removeAttr("length");
            add_string_attribute(lhandle, "length", "FOO");
        }
    }
    expect_error(path, "hello", "failed to load 'seeds' list");

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = combine_opener(fhandle, "hello", 0, version);
        list_opener(ghandle, "seeds", 1, version);
    }
    expect_error(path, "hello", "missing elements in the 'seeds' list");

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = combine_opener(fhandle, "hello", 0, version);
        auto lhandle = list_opener(ghandle, "seeds", 1, version);
        auto shandle = lhandle.createGroup("0");
        add_string_attribute(shandle, "delayed_type", "FOOBAR");
    }
    expect_error(path, "hello", "failed to validate 'seeds/0'");

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = combine_opener(fhandle, "hello", 0, version);
        auto lhandle = list_opener(ghandle, "seeds", 2, version);
        mock_array_opener<int>(lhandle, "0", { 13, 10 }, version, "BOOLEAN");
        mock_array_opener<int>(lhandle, "1", { 13, 10, 5 }, version, "BOOLEAN");
    }
    expect_error(path, "hello", "dimensionality mismatch");

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = combine_opener(fhandle, "hello", 0, version);
        auto lhandle = list_opener(ghandle, "seeds", 2, version);
        mock_array_opener<int>(lhandle, "0", { 13, 10 }, version, "BOOLEAN");
        mock_array_opener<int>(lhandle, "1", { 5, 15 }, version, "BOOLEAN");
    }
    expect_error(path, "hello", "inconsistent dimension extents");

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = combine_opener(fhandle, "hello", 1, version);
        auto lhandle = list_opener(ghandle, "seeds", 2, version);
        mock_array_opener<int>(lhandle, "0", { 13, 10 }, version, "STRING");
        mock_array_opener<int>(lhandle, "1", { 13, 15 }, version, "INTEGER");
    }
    expect_error(path, "hello", "contain strings");
}

INSTANTIATE_TEST_SUITE_P(
    Combine,
    CombineErrorTest,
    spawn_all_versions()
);
