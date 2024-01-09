#include <gtest/gtest.h>
#include "chihaya/chihaya.hpp"
#include "utils.h"

class CombineTest : public ::testing::TestWithParam<int> {
protected:
    void add_along(H5::Group& handle, int along, int version) {
        if (version < 1100000) {
            add_numeric_scalar(handle, "along", along, H5::PredType::NATIVE_INT);
        } else {
            add_numeric_scalar(handle, "along", along, H5::PredType::NATIVE_UINT32);
        }
    }
};

TEST_P(CombineTest, Simple) {
    auto version = GetParam();
    std::string path = "Test_combine.h5";

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = operation_opener(fhandle, "hello", "combine");
        add_version_string(ghandle, version);
        add_along(ghandle, 0, version);

        auto lhandle = list_opener(ghandle, "seeds", 2, version);
        CustomArrayOptions opt(version, "FLOAT");
        custom_array_opener(lhandle, "0", { 13, 19 }, opt);
        custom_array_opener(lhandle, "1", { 20, 19 }, opt); 
    }

    auto output = chihaya::validate(path, "hello"); 
    EXPECT_EQ(output.type, chihaya::FLOAT);
    const auto& dims = output.dimensions;
    EXPECT_EQ(dims[0], 33);
    EXPECT_EQ(dims[1], 19);
}

TEST_P(CombineTest, Mixed) {
    auto version = GetParam();
    std::string path = "Test_combine.h5";

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = operation_opener(fhandle, "hello", "combine");
        add_along(ghandle, 1, version);
        add_version_string(ghandle, version);

        auto lhandle = list_opener(ghandle, "seeds", 3, version);
        custom_array_opener(lhandle, "0", { 13, 10 }, CustomArrayOptions(version, "BOOLEAN"));
        custom_array_opener(lhandle, "1", { 13, 20 }, CustomArrayOptions(version, "INTEGER")); 
        custom_array_opener(lhandle, "2", { 13, 30 }, CustomArrayOptions(version, "BOOLEAN")); 
    }

    auto output = chihaya::validate(path, "hello"); 
    EXPECT_EQ(output.type, chihaya::INTEGER);
    const auto& dims = output.dimensions;
    EXPECT_EQ(dims[0], 13);
    EXPECT_EQ(dims[1], 60);
}

TEST_P(CombineTest, AlongErrors) {
    auto version = GetParam();
    std::string path = "Test_combine.h5";

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = operation_opener(fhandle, "hello", "combine");
        add_version_string(ghandle, version);
    }
    expect_error([&]() -> void { chihaya::validate(path, "hello"); }, "expected a dataset at 'along'");

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = operation_opener(fhandle, "hello", "combine");
        add_version_string(ghandle, version);
        add_numeric_vector<int>(ghandle, "along", { 1 }, H5::PredType::NATIVE_INT);
    }
    expect_error([&]() -> void { chihaya::validate(path, "hello"); }, "'along' should be a scalar dataset");

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = operation_opener(fhandle, "hello", "combine");
        add_version_string(ghandle, version);
        add_numeric_scalar(ghandle, "along", -1, H5::PredType::NATIVE_INT);
    }
    if (version >= 1100000) {
        expect_error([&]() -> void { chihaya::validate(path, "hello"); }, "64-bit unsigned integer");
    } else {
        expect_error([&]() -> void { chihaya::validate(path, "hello"); }, "'along' should be non-negative");
    }

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = operation_opener(fhandle, "hello", "combine");
        add_version_string(ghandle, version);
        add_along(ghandle, 2, version);

        auto lhandle = list_opener(ghandle, "seeds", 1, version);
        custom_array_opener(lhandle, "0", { 13, 10 }, CustomArrayOptions(version, "BOOLEAN"));
    }
    expect_error([&]() -> void { chihaya::validate(path, "hello"); }, "'along' should be less than the seed dimensionality");
}

TEST_P(CombineTest, SeedErrors) {
    auto version = GetParam();
    std::string path = "Test_combine.h5";

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = operation_opener(fhandle, "hello", "combine");
        add_version_string(ghandle, version);
        add_along(ghandle, 0, version);
    }
    expect_error([&]() -> void { chihaya::validate(path, "hello"); }, "expected a group at 'seeds'");

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = operation_opener(fhandle, "hello", "combine");
        add_version_string(ghandle, version);
        add_along(ghandle, 0, version);

        auto lhandle = list_opener(ghandle, "seeds", 1, version);
        if (version < 1100000) {
            lhandle.removeAttr("delayed_length");
        } else {
            lhandle.removeAttr("length");
        }
    }
    expect_error([&]() -> void { chihaya::validate(path, "hello"); }, "failed to load 'seeds' list");

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = operation_opener(fhandle, "hello", "combine");
        add_version_string(ghandle, version);
        add_along(ghandle, 0, version);
        list_opener(ghandle, "seeds", 1, version);
    }
    expect_error([&]() -> void { chihaya::validate(path, "hello"); }, "missing elements in the 'seeds' list");

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = operation_opener(fhandle, "hello", "combine");
        add_along(ghandle, 0, version);
        add_version_string(ghandle, version);

        auto lhandle = list_opener(ghandle, "seeds", 2, version);
        CustomArrayOptions opt(version, "BOOLEAN");
        custom_array_opener(lhandle, "0", { 13, 10 }, opt);
        custom_array_opener(lhandle, "1", { 13, 10, 5 }, opt);
    }
    expect_error([&]() -> void { chihaya::validate(path, "hello"); }, "dimensionality mismatch");

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = operation_opener(fhandle, "hello", "combine");
        add_along(ghandle, 0, version);
        add_version_string(ghandle, version);

        auto lhandle = list_opener(ghandle, "seeds", 2, version);
        CustomArrayOptions opt(version, "BOOLEAN");
        custom_array_opener(lhandle, "0", { 13, 10 }, opt);
        custom_array_opener(lhandle, "1", { 5, 15 }, opt);
    }
    expect_error([&]() -> void { chihaya::validate(path, "hello"); }, "inconsistent dimension extents");
}

INSTANTIATE_TEST_SUITE_P(
    Combine,
    CombineTest,
    ::testing::Values(0, 1000000, 1100000)
);
