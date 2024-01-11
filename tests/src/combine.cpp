#include <gtest/gtest.h>
#include "chihaya/chihaya.hpp"
#include "utils.h"

class CombineTest : public ::testing::TestWithParam<int> {
public:
    CombineTest() : path("Test_combine.h5") {}
protected:
    std::string path;

    static H5::Group combine_opener(H5::Group& handle, const std::string& name, int along, int version) {
        auto ghandle = operation_opener(handle, name, "combine");
        add_version_string(ghandle, version);

        if (version < 1100000) {
            add_numeric_scalar(ghandle, "along", along, H5::PredType::NATIVE_INT);
        } else {
            add_numeric_scalar(ghandle, "along", along, H5::PredType::NATIVE_UINT32);
        }

        return ghandle;
    }
};

TEST_P(CombineTest, Simple) {
    auto version = GetParam();

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = combine_opener(fhandle, "hello", 0, version);
        auto lhandle = list_opener(ghandle, "seeds", 2, version);
        mock_array_opener(lhandle, "0", { 13, 19 }, version, "FLOAT");
        mock_array_opener(lhandle, "1", { 20, 19 }, version, "FLOAT"); 
    }
    {
        auto output = test_validate(path, "hello"); 
        EXPECT_EQ(output.type, chihaya::FLOAT);
        const auto& dims = output.dimensions;
        EXPECT_EQ(dims[0], 33);
        EXPECT_EQ(dims[1], 19);
    }

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = combine_opener(fhandle, "hello", 1, version);
        auto lhandle = list_opener(ghandle, "seeds", 2, version);
        mock_array_opener(lhandle, "0", { 10, 52 }, version, "STRING");
        mock_array_opener(lhandle, "1", { 10, 12 }, version, "STRING"); 
    }
    {
        auto output = test_validate(path, "hello"); 
        EXPECT_EQ(output.type, chihaya::STRING);
        const auto& dims = output.dimensions;
        EXPECT_EQ(dims[0], 10);
        EXPECT_EQ(dims[1], 64);
    }
}

TEST_P(CombineTest, Mixed) {
    auto version = GetParam();

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = combine_opener(fhandle, "hello", 1, version);
        auto lhandle = list_opener(ghandle, "seeds", 3, version);
        mock_array_opener(lhandle, "0", { 13, 10 }, version, "BOOLEAN");
        mock_array_opener(lhandle, "1", { 13, 20 }, version, "INTEGER"); 
        mock_array_opener(lhandle, "2", { 13, 30 }, version, "BOOLEAN"); 
    }
    auto output = test_validate(path, "hello"); 
    EXPECT_EQ(output.type, chihaya::INTEGER);
    const auto& dims = output.dimensions;
    EXPECT_EQ(dims[0], 13);
    EXPECT_EQ(dims[1], 60);
}

TEST_P(CombineTest, AlongErrors) {
    auto version = GetParam();

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = combine_opener(fhandle, "hello", 0, version);
        ghandle.unlink("along");
    }
    expect_error(path, "hello", "expected a dataset at 'along'");

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = combine_opener(fhandle, "hello", 0, version);
        ghandle.unlink("along");
        add_numeric_vector<int>(ghandle, "along", { 1 }, H5::PredType::NATIVE_INT);
    }
    expect_error(path, "hello", "'along' should be a scalar dataset");

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = combine_opener(fhandle, "hello", 0, version);
        ghandle.unlink("along");
        add_numeric_scalar(ghandle, "along", -1, H5::PredType::NATIVE_INT);
    }
    if (version >= 1100000) {
        expect_error(path, "hello", "64-bit unsigned integer");
    } else {
        expect_error(path, "hello", "'along' should be non-negative");
    }

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = combine_opener(fhandle, "hello", 2, version);
        auto lhandle = list_opener(ghandle, "seeds", 1, version);
        mock_array_opener(lhandle, "0", { 13, 10 }, version, "BOOLEAN");
    }
    expect_error(path, "hello", "'along' should be less than the seed dimensionality");
}

TEST_P(CombineTest, SeedErrors) {
    auto version = GetParam();

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        combine_opener(fhandle, "hello", 0, version);
    }
    expect_error(path, "hello", "expected a group at 'seeds'");

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = combine_opener(fhandle, "hello", 0, version);
        auto lhandle = list_opener(ghandle, "seeds", 1, version);
        if (version < 1100000) {
            lhandle.removeAttr("delayed_length");
        } else {
            lhandle.removeAttr("length");
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
        lhandle.createGroup("0");
    }
    expect_error(path, "hello", "failed to validate 'seeds/0'");

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = combine_opener(fhandle, "hello", 0, version);
        auto lhandle = list_opener(ghandle, "seeds", 2, version);
        mock_array_opener(lhandle, "0", { 13, 10 }, version, "BOOLEAN");
        mock_array_opener(lhandle, "1", { 13, 10, 5 }, version, "BOOLEAN");
    }
    expect_error(path, "hello", "dimensionality mismatch");

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = combine_opener(fhandle, "hello", 0, version);
        auto lhandle = list_opener(ghandle, "seeds", 2, version);
        mock_array_opener(lhandle, "0", { 13, 10 }, version, "BOOLEAN");
        mock_array_opener(lhandle, "1", { 5, 15 }, version, "BOOLEAN");
    }
    expect_error(path, "hello", "inconsistent dimension extents");

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = combine_opener(fhandle, "hello", 1, version);
        auto lhandle = list_opener(ghandle, "seeds", 2, version);
        mock_array_opener(lhandle, "0", { 13, 10 }, version, "STRING");
        mock_array_opener(lhandle, "1", { 13, 15 }, version, "INTEGER");
    }
    expect_error(path, "hello", "contain strings");
}

INSTANTIATE_TEST_SUITE_P(
    Combine,
    CombineTest,
    ::testing::Values(0, 1000000, 1100000)
);
