#include <gtest/gtest.h>

#include "H5Cpp.h"
#include "ritsuko/ritsuko.hpp"
#include "chihaya/utils_subset.hpp"

#include "utils.h"

class ValidateSubsetIndexListTest : public ::testing::TestWithParam<ritsuko::Version> {};

static std::pair<std::size_t, std::size_t> make_pair_z(int f, int s) {
    return std::pair<std::size_t, std::size_t>(f, s);
}

TEST_P(ValidateSubsetIndexListTest, NoOp) {
    auto path = define_test_path("utils_subset");
    auto version = GetParam();

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        list_opener(fhandle, "index", 2, version);
    }

    H5::H5File fhandle(path, H5F_ACC_RDONLY);
    auto collected = chihaya::validate_subset_index_list(fhandle.openGroup("index"), { 10, 20 }, version, 1000);
    EXPECT_TRUE(collected.empty());
}

TEST_P(ValidateSubsetIndexListTest, NonEmpty) {
    auto path = define_test_path("utils_subset");
    auto version = GetParam();

    // One subset.
    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto lhandle = list_opener(fhandle, "index", 3, version);
        add_numeric_vector<int>(lhandle, "2", { 1, 1, 2, 3, 5, 8 }, H5::PredType::NATIVE_UINT8);
    }
    {
        H5::H5File fhandle(path, H5F_ACC_RDONLY);
        auto collected = chihaya::validate_subset_index_list(fhandle.openGroup("index"), { 20, 5, 10 }, version, 1000);
        ASSERT_EQ(collected.size(), 1);
        EXPECT_EQ(collected[0], make_pair_z(2, 6));
    }

    // Two subsets.
    {
        H5::H5File fhandle(path, H5F_ACC_RDWR);
        auto lhandle = fhandle.openGroup("index");
        add_numeric_vector<int>(lhandle, "0", { 11, 13, 10, 12, 19, 18, 14 }, H5::PredType::NATIVE_UINT16);
    }
    {
        H5::H5File fhandle(path, H5F_ACC_RDONLY);
        auto collected = chihaya::validate_subset_index_list(fhandle.openGroup("index"), { 20, 5, 10 }, version, 1000);
        ASSERT_EQ(collected.size(), 2);
        EXPECT_EQ(collected[0], make_pair_z(0, 7));
        EXPECT_EQ(collected[1], make_pair_z(2, 6));
    }

    // All subsets.
    {
        H5::H5File fhandle(path, H5F_ACC_RDWR);
        auto lhandle = fhandle.openGroup("index");
        add_numeric_vector<int>(lhandle, "1", {}, H5::PredType::NATIVE_UINT32);
    }
    {
        H5::H5File fhandle(path, H5F_ACC_RDONLY);
        auto collected = chihaya::validate_subset_index_list(fhandle.openGroup("index"), { 20, 5, 10 }, version, 1000);
        ASSERT_EQ(collected.size(), 3);
        EXPECT_EQ(collected[0], make_pair_z(0, 7));
        EXPECT_EQ(collected[1], make_pair_z(1, 0));
        EXPECT_EQ(collected[2], make_pair_z(2, 6));
    }
}

TEST_P(ValidateSubsetIndexListTest, ListError) {
    auto path = define_test_path("utils_subset");
    auto version = GetParam();

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto lhandle = list_opener(fhandle, "index", 2, version);
        add_numeric_vector<int>(lhandle, "foo", { 1, 3, 0, 2, 9 }, H5::PredType::NATIVE_DOUBLE);
    }
    {
        H5::H5File fhandle(path, H5F_ACC_RDONLY);
        expect_error([&]() -> void {
            chihaya::validate_subset_index_list(fhandle.openGroup("index"), { 2, 10 }, version, 1000);
        }, "failed to load");
    }

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto lhandle = list_opener(fhandle, "index", 2, version);
    }
    {
        H5::H5File fhandle(path, H5F_ACC_RDONLY);
        expect_error([&]() -> void {
            chihaya::validate_subset_index_list(fhandle.openGroup("index"), { 2, 10, 5 }, version, 1000);
        }, "equal to number of dimensions");
    }
}

TEST_P(ValidateSubsetIndexListTest, IndexError) {
    auto path = define_test_path("utils_subset");
    auto version = GetParam();

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto lhandle = list_opener(fhandle, "index", 2, version);
        add_numeric_scalar(lhandle, "0", 2, H5::PredType::NATIVE_UINT8);
    }
    {
        H5::H5File fhandle(path, H5F_ACC_RDONLY);
        expect_error([&]() -> void {
            chihaya::validate_subset_index_list(fhandle.openGroup("index"), { 2, 10 }, version, 1000);
        }, "1-dimensional");
    }

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto lhandle = list_opener(fhandle, "index", 2, version);
        add_numeric_vector<int>(lhandle, "1", { 1, 3, 0, 2, 9 }, H5::PredType::NATIVE_DOUBLE);
    }
    if (version.lt(1, 1, 0)) {
        H5::H5File fhandle(path, H5F_ACC_RDONLY);
        expect_error([&]() -> void {
            chihaya::validate_subset_index_list(fhandle.openGroup("index"), { 2, 10 }, version, 1000);
        }, "expected an integer type");
    } else {
        H5::H5File fhandle(path, H5F_ACC_RDONLY);
        expect_error([&]() -> void {
            chihaya::validate_subset_index_list(fhandle.openGroup("index"), { 2, 10 }, version, 1000);
        }, "64-bit unsigned integer");
    }

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto lhandle = list_opener(fhandle, "index", 2, version);
        add_numeric_vector<int>(lhandle, "1", { 1, 3, 0, -2, 9 }, H5::PredType::NATIVE_INT);
    }
    if (version.lt(1, 1, 0)) {
        H5::H5File fhandle(path, H5F_ACC_RDONLY);
        expect_error([&]() -> void {
            chihaya::validate_subset_index_list(fhandle.openGroup("index"), { 2, 10 }, version, 1000);
        }, "non-negative");
    } else {
        H5::H5File fhandle(path, H5F_ACC_RDONLY);
        expect_error([&]() -> void {
            chihaya::validate_subset_index_list(fhandle.openGroup("index"), { 2, 10 }, version, 1000);
        }, "64-bit unsigned integer");
    }

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto lhandle = list_opener(fhandle, "index", 2, version);
        add_numeric_vector<int>(lhandle, "0", { 1, 3, 0, 5, 9 }, H5::PredType::NATIVE_UINT64);
    }
    H5::H5File fhandle(path, H5F_ACC_RDONLY);
    expect_error([&]() -> void {
        chihaya::validate_subset_index_list(fhandle.openGroup("index"), { 2, 10 }, version, 1000);
    }, "out of range");
}

TEST_P(ValidateSubsetIndexListTest, IndexErrorChunked) {
    auto path = define_test_path("utils_subset");
    auto version = GetParam();

    std::vector<int> subsets(1000);
    const int extent = 11;
    for (int i = 0; i < 1000; ++i) {
        subsets[i] = i % extent;
    }
    const hsize_t dim = subsets.size();
    H5::DataSpace dspace(1, &dim);

    // Here, the aim is to check that we iterate across the subset vector correctly.
    const hsize_t chunk_size = dim / 11;
    H5::DSetCreatPropList dcpl;
    dcpl.setChunk(1, &chunk_size);
    dcpl.setDeflate(6);

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto lhandle = list_opener(fhandle, "index", 3, version);
        auto dhandle = lhandle.createDataSet("2", H5::PredType::NATIVE_UINT8, dspace, dcpl);
        dhandle.write(subsets.data(), H5::PredType::NATIVE_INT);
    }
    {
        H5::H5File fhandle(path, H5F_ACC_RDONLY);
        auto collected = chihaya::validate_subset_index_list(fhandle.openGroup("index"), { 2, 3, extent }, version, 1000);
        ASSERT_EQ(collected.size(), 1);
        EXPECT_EQ(collected[0], make_pair_z(2, subsets.size()));
    }

    for (int scenario = 0; scenario < 3; ++scenario) {
        // Ensuring that we can catch out-of-range errors anywhere within the subset vector.
        auto modified = subsets;
        if (scenario == 0) {
            modified[0] = extent;
        } else if (scenario == 1) {
            modified[500] = extent;
        } else {
            modified.back() = extent;
        }

        {
            H5::H5File fhandle(path, H5F_ACC_RDWR);
            auto lhandle = fhandle.openGroup("index");
            auto dhandle = lhandle.openDataSet("2");
            dhandle.write(modified.data(), H5::PredType::NATIVE_INT);
        }

        H5::H5File fhandle(path, H5F_ACC_RDONLY);
        expect_error([&]() -> void {
            chihaya::validate_subset_index_list(fhandle.openGroup("index"), { 2, 3, extent }, version, 1000);
        }, "out of range");
    }
}

INSTANTIATE_TEST_SUITE_P(
    ValidateSubsetIndexList,
    ValidateSubsetIndexListTest,
    spawn_all_versions()
);
