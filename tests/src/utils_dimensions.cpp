#include <gtest/gtest.h>

#include <vector>
#include <cstddef>

#include "H5Cpp.h"
#include "ritsuko/ritsuko.hpp"
#include "chihaya/utils_dimensions.hpp"

#include "utils.h"

TEST(AreDimensionsEqual, Basic) {
    std::vector<int> x{ 1, 2, 3 };
    EXPECT_TRUE(chihaya::are_dimensions_equal(x, x));

    std::vector<int> y{ 10, 20 };
    EXPECT_FALSE(chihaya::are_dimensions_equal(x, y));

    y.push_back(30);
    EXPECT_FALSE(chihaya::are_dimensions_equal(x, y));
}

/***********************************/

TEST(LoadDimensionsFromUint64Contents, Basic) {
    auto path = define_test_path("utils_dimensions");

    std::vector<std::size_t> dims{ 23, 41 };
    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        add_numeric_vector(fhandle, "dims", dims, H5::PredType::NATIVE_UINT32);
    }

    // Force a copy.
    {
        H5::H5File fhandle(path, H5F_ACC_RDONLY);
        auto output = chihaya::load_dimensions_from_uint64_contents<int>(fhandle.openDataSet("dims"), 2);
        EXPECT_EQ(output, std::vector<int>(dims.begin(), dims.end()));
    }

    // No need for a copy.
    {
        H5::H5File fhandle(path, H5F_ACC_RDONLY);
        auto output = chihaya::load_dimensions_from_uint64_contents<hsize_t>(fhandle.openDataSet("dims"), 2);
        EXPECT_EQ(output, std::vector<hsize_t>(dims.begin(), dims.end()));
    }
}

/***********************************/

class LoadAlongTest : public ::testing::TestWithParam<ritsuko::Version> {};

TEST_P(LoadAlongTest, Okay) {
    auto path = define_test_path("utils_dimensions");
    auto version = GetParam();

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        add_numeric_scalar(fhandle, "along", 99, H5::PredType::NATIVE_UINT8);
    }

    H5::H5File fhandle(path, H5F_ACC_RDONLY);
    EXPECT_EQ(chihaya::load_along(fhandle, version), 99);
}

TEST_P(LoadAlongTest, Error) {
    auto path = define_test_path("utils_dimensions");
    auto version = GetParam();

    // Not scalar.
    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        add_numeric_vector<int>(fhandle, "along", { 1 }, H5::PredType::NATIVE_INT);
    }
    {
        H5::H5File fhandle(path, H5F_ACC_RDONLY);
        expect_error([&]() -> void { chihaya::load_along(fhandle, version); }, "scalar");
    }

    // Signed type.
    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        add_numeric_scalar(fhandle, "along", -1, H5::PredType::NATIVE_INT);
    }
    if (version.lt(1, 1, 0)) {
        H5::H5File fhandle(path, H5F_ACC_RDONLY);
        expect_error([&]() -> void { chihaya::load_along(fhandle, version); }, "non-negative");
    } else {
        H5::H5File fhandle(path, H5F_ACC_RDONLY);
        expect_error([&]() -> void { chihaya::load_along(fhandle, version); }, "64-bit unsigned integer");
    }
}

INSTANTIATE_TEST_SUITE_P(
    LoadAlong,
    LoadAlongTest,
    spawn_all_versions()
);

/***********************************/

class ValidateDimnamesInternalTest : public ::testing::TestWithParam<ritsuko::Version> {};

TEST_P(ValidateDimnamesInternalTest, None) {
    auto path = define_test_path("utils_dimensions");
    auto version = GetParam();

    std::vector<std::size_t> dimensions{ 10, 15, 5};
    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        list_opener(fhandle, "dimnames", dimensions.size(), version);
    }

    H5::H5File fhandle(path, H5F_ACC_RDONLY);
    chihaya::validate_dimnames_internal(fhandle, dimensions, version, 1000);
}

TEST_P(ValidateDimnamesInternalTest, Partial) {
    auto path = define_test_path("utils_dimensions");
    auto version= GetParam();

    std::vector<std::size_t> dimensions{ 10, 15, 5 };
    for (int i = 0; i < 3; ++i) {
        {
            H5::H5File fhandle(path, H5F_ACC_TRUNC);
            auto lhandle = list_opener(fhandle, "dimnames", dimensions.size(), version);
            add_string_vector(lhandle, std::to_string(i), dimensions[i], /* strlen = */ 5);
        }

        H5::H5File fhandle(path, H5F_ACC_RDONLY);
        chihaya::validate_dimnames_internal(fhandle, dimensions, version, 1000);
    }
}

TEST_P(ValidateDimnamesInternalTest, Full) {
    auto path = define_test_path("utils_dimensions");
    auto version = GetParam();

    std::vector<std::size_t> dimensions{ 12, 20 };
    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto lhandle = list_opener(fhandle, "dimnames", dimensions.size(), version);
        add_string_vector(lhandle, "0", 12, /* strlen = */ 5);
        add_string_vector(lhandle, "1", 20, /* strlen = */ 2);
    }

    H5::H5File fhandle(path, H5F_ACC_RDONLY);
    chihaya::validate_dimnames_internal(fhandle, dimensions, version, 1000);
}

TEST_P(ValidateDimnamesInternalTest, Errors) {
    auto path = define_test_path("utils_dimensions");
    auto version = GetParam();

    std::vector<std::size_t> dimensions{ 5, 98 };

    // Check that the list is actually validated.
    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto lhandle = list_opener(fhandle, "dimnames", dimensions.size(), version);
        lhandle.createGroup("BLAH");
    }
    {
        H5::H5File fhandle(path, H5F_ACC_RDONLY);
        expect_error([&]() -> void { chihaya::validate_dimnames_internal(fhandle, dimensions, version, 1000); }, "not a valid name");
    }

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        list_opener(fhandle, "dimnames", 1, version);
    }
    {
        H5::H5File fhandle(path, H5F_ACC_RDONLY);
        expect_error([&]() -> void { chihaya::validate_dimnames_internal(fhandle, dimensions, version, 1000); }, "length of 'dimnames' list");
    }

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto lhandle = list_opener(fhandle, "dimnames", dimensions.size(), version);
        add_string_scalar(lhandle, "1", "FOOBAR");
    }
    {
        H5::H5File fhandle(path, H5F_ACC_RDONLY);
        expect_error([&]() -> void { chihaya::validate_dimnames_internal(fhandle, dimensions, version, 1000); }, "1-dimensional");
    }

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto lhandle = list_opener(fhandle, "dimnames", dimensions.size(), version);
        add_numeric_vector<int>(lhandle, "0", {1}, H5::PredType::NATIVE_INT32);
    }
    {
        H5::H5File fhandle(path, H5F_ACC_RDONLY);
        expect_error([&]() -> void { chihaya::validate_dimnames_internal(fhandle, dimensions, version, 1000); }, "UTF-8 strings");
    }

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto lhandle = list_opener(fhandle, "dimnames", dimensions.size(), version);
        add_string_vector(lhandle, "1", 15, /* strlen = */ 3);
    }
    {
        H5::H5File fhandle(path, H5F_ACC_RDONLY);
        expect_error([&]() -> void { chihaya::validate_dimnames_internal(fhandle, dimensions, version, 1000); }, "length equal to the extent");
    }

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto lhandle = list_opener(fhandle, "dimnames", dimensions.size(), version);
        add_string_vector(lhandle, "1", dimensions[1], /* strlen = */ H5T_VARIABLE);
    }
    {
        H5::H5File fhandle(path, H5F_ACC_RDONLY);
        expect_error([&]() -> void { chihaya::validate_dimnames_internal(fhandle, dimensions, version, 1000); }, "NULL");
    }
}

INSTANTIATE_TEST_SUITE_P(
    ValidateDimnamesInternal,
    ValidateDimnamesInternalTest,
    spawn_all_versions()
);
