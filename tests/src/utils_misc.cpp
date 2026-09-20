#include <gtest/gtest.h>

#include "H5Cpp.h"
#include "ritsuko/ritsuko.hpp"
#include "chihaya/utils_misc.hpp"

#include "utils.h"

class ValidateMissingPlaceholderTest : public ::testing::TestWithParam<ritsuko::Version> {};

TEST_P(ValidateMissingPlaceholderTest, Absent) {
    const auto path = define_test_path("utils_misc");
    const auto version = GetParam();

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        add_numeric_scalar<int>(fhandle, "none", 1, H5::PredType::NATIVE_INT);
    }

    H5::H5File fhandle(path, H5F_ACC_RDONLY);
    chihaya::validate_missing_placeholder(fhandle.openDataSet("none"), version);
}

TEST_P(ValidateMissingPlaceholderTest, Integer) {
    const auto path = define_test_path("utils_misc");
    const auto version = GetParam();

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ihandle = add_numeric_scalar<int>(fhandle, "inty", 1, H5::PredType::NATIVE_INT32);
        if (version.lt(1, 1, 0)) {
            add_numeric_missing_placeholder<int>(ihandle, 3, H5::PredType::NATIVE_INT8);
        } else {
            add_numeric_missing_placeholder<int>(ihandle, 3, H5::PredType::NATIVE_INT32);
        }
    }

    H5::H5File fhandle(path, H5F_ACC_RDONLY);
    chihaya::validate_missing_placeholder(fhandle.openDataSet("inty"), version);
}

TEST_P(ValidateMissingPlaceholderTest, Float) {
    const auto path = define_test_path("utils_misc");
    const auto version = GetParam();

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto dhandle = add_numeric_scalar<int>(fhandle, "floaty", 1.0, H5::PredType::NATIVE_DOUBLE);
        add_numeric_missing_placeholder<int>(dhandle, 3.0, H5::PredType::NATIVE_DOUBLE);
    }

    H5::H5File fhandle(path, H5F_ACC_RDONLY);
    chihaya::validate_missing_placeholder(fhandle.openDataSet("floaty"), version);
}

TEST_P(ValidateMissingPlaceholderTest, String) {
    const auto path = define_test_path("utils_misc");
    const auto version = GetParam();

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto shandle = add_string_scalar(fhandle, "stringy", "FOO");
        add_string_missing_placeholder(shandle, "BAR");
    }

    H5::H5File fhandle(path, H5F_ACC_RDONLY);
    chihaya::validate_missing_placeholder(fhandle.openDataSet("stringy"), version);
}

TEST_P(ValidateMissingPlaceholderTest, ShapeError) {
    const auto path = define_test_path("utils_misc");
    const auto version = GetParam();
    if (version.lt(1, 0, 0)) {
        return;
    }

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ihandle = add_numeric_vector<int>(fhandle, "inty", { 1, 2, 3 }, H5::PredType::NATIVE_INT32);
        constexpr hsize_t one = 1;
        ihandle.createAttribute("missing_placeholder", H5::PredType::NATIVE_INT32, H5::DataSpace(1, &one));
    }

    H5::H5File fhandle(path, H5F_ACC_RDONLY);
    expect_error([&]() -> void {
        chihaya::validate_missing_placeholder(fhandle.openDataSet("inty"), version);
    }, "scalar");
}

TEST_P(ValidateMissingPlaceholderTest, StringTypeError) {
    const auto path = define_test_path("utils_misc");
    const auto version = GetParam();
    if (version.lt(1, 0, 0)) {
        return;
    }

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto shandle = add_string_vector(fhandle, "stringy", 20);
        add_numeric_missing_placeholder<int>(shandle, 3.0, H5::PredType::NATIVE_DOUBLE);
    }

    H5::H5File fhandle(path, H5F_ACC_RDONLY);
    expect_error([&]() -> void {
        chihaya::validate_missing_placeholder(fhandle.openDataSet("stringy"), version);
    }, "string datatype class");
}

TEST_P(ValidateMissingPlaceholderTest, NumericTypeError) {
    const auto path = define_test_path("utils_misc");
    const auto version = GetParam();
    if (version.lt(1, 0, 0)) {
        return;
    }

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto dhandle = add_numeric_vector<int>(fhandle, "floaty", { 1, 2, 3 }, H5::PredType::NATIVE_DOUBLE);
        if (version.lt(1, 1, 0)) {
            add_numeric_missing_placeholder<int>(dhandle, 3, H5::PredType::NATIVE_INT8);
        } else {
            add_numeric_missing_placeholder<int>(dhandle, 3, H5::PredType::NATIVE_FLOAT);
        }
    }

    H5::H5File fhandle(path, H5F_ACC_RDONLY);
    if (version.lt(1, 1, 0)) {
        expect_error([&]() -> void {
            chihaya::validate_missing_placeholder(fhandle.openDataSet("floaty"), version);
        }, "same datatype class as");
    } else {
        expect_error([&]() -> void {
            chihaya::validate_missing_placeholder(fhandle.openDataSet("floaty"), version);
        }, "same datatype as");
    }
}

INSTANTIATE_TEST_SUITE_P(
    ValidateMissingPlaceholder,
    ValidateMissingPlaceholderTest,
    spawn_all_versions()
);

/***************************************************/

class FetchSeedTest : public ::testing::TestWithParam<ritsuko::Version> {};

TEST_P(FetchSeedTest, Okay) {
    auto version = GetParam();
    const auto path = define_test_path("utils_misc");
    chihaya::Options options;

    std::vector<std::size_t> dims{ 13, 14 };
    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        mock_array_opener(fhandle, "seed", dims, version, "INTEGER");
    }

    H5::H5File fhandle(path, H5F_ACC_RDONLY);
    auto deets = chihaya::fetch_seed(fhandle, "seed", version, options); 
    EXPECT_EQ(deets.type, chihaya::INTEGER);
    EXPECT_EQ(deets.dimensions, dims);
}

TEST_P(FetchSeedTest, Error) {
    auto version = GetParam();
    const auto path = define_test_path("utils_misc");
    chihaya::Options options;

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = fhandle.createGroup("seed");
        add_string_attribute(ghandle, "delayed_type", "some_random_thing");
    }

    H5::H5File fhandle(path, H5F_ACC_RDONLY);
    expect_error([&]() -> void {
        chihaya::fetch_seed(fhandle, "seed", version, options);
    }, "failed to validate");
}

INSTANTIATE_TEST_SUITE_P(
    FetchSeed,
    FetchSeedTest,
    spawn_all_versions()
);

/***************************************************/

TEST(LoadScalarStringDataset, Basic) {
    const auto path = define_test_path("utils_misc");

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        add_string_scalar(fhandle, "foo", "bar");
        add_string_vector(fhandle, "whee", 20);
        add_numeric_scalar<int>(fhandle, "stuff", 20, H5::PredType::NATIVE_INT);
    }

    H5::H5File fhandle(path, H5F_ACC_RDONLY);
    EXPECT_EQ(chihaya::load_scalar_string_dataset(fhandle, "foo"), "bar");
    expect_error([&]() -> void { chihaya::load_scalar_string_dataset(fhandle, "whee"); }, "scalar");
    expect_error([&]() -> void { chihaya::load_scalar_string_dataset(fhandle, "stuff"); }, "string");
}

TEST(LoadScalarStringAttribute, Basic) {
    const auto path = define_test_path("utils_misc");

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = fhandle.createGroup("blah");
        add_string_attribute(ghandle, "foo", "bar");
        constexpr hsize_t dim = 20;
        ghandle.createAttribute("whee", H5::StrType(0, H5T_VARIABLE), H5::DataSpace(1, &dim));
        ghandle.createAttribute("stuff", H5::PredType::NATIVE_UINT8, H5S_SCALAR);
    }

    H5::H5File fhandle(path, H5F_ACC_RDONLY);
    auto ghandle = fhandle.openGroup("blah");
    EXPECT_EQ(chihaya::load_scalar_string_attribute(ghandle, "foo"), "bar");
    expect_error([&]() -> void { chihaya::load_scalar_string_attribute(ghandle, "whee"); }, "scalar");
    expect_error([&]() -> void { chihaya::load_scalar_string_attribute(ghandle, "stuff"); }, "string");
}
