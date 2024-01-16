#include <gtest/gtest.h>
#include "chihaya/utils_type.hpp"
#include "utils.h"

TEST(UtilsMisc, AreDimensionsEqual) {
    std::vector<int> d1 { 1 };
    std::vector<int> d2 { 1, 4 };
    EXPECT_FALSE(chihaya::internal_misc::are_dimensions_equal(d1, d2));

    d1.push_back(2);
    EXPECT_FALSE(chihaya::internal_misc::are_dimensions_equal(d1, d2));

    d1.back() = 4;
    EXPECT_TRUE(chihaya::internal_misc::are_dimensions_equal(d1, d2));
}

TEST(UtilsMisc, ValidateMissingPlaceholder) {
    const char * path = "Test_utils_misc.h5";

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        add_numeric_scalar<int>(fhandle, "none", 1, H5::PredType::NATIVE_INT);

        auto ihandle = add_numeric_scalar<int>(fhandle, "inty", 1, H5::PredType::NATIVE_INT32);
        add_numeric_missing_placeholder<int>(ihandle, 3, H5::PredType::NATIVE_UINT8);

        auto ihandle2 = add_numeric_scalar<int>(fhandle, "inty2", 1, H5::PredType::NATIVE_INT32);
        add_numeric_missing_placeholder<int>(ihandle2, 3, H5::PredType::NATIVE_INT32);

        auto shandle = add_string_scalar(fhandle, "stringy", "FOO");
        add_string_missing_placeholder(shandle, "BAR");
    }

    {
        H5::H5File fhandle(path, H5F_ACC_RDONLY);

        auto nhandle = fhandle.openDataSet("none");
        chihaya::internal_misc::validate_missing_placeholder(nhandle, ritsuko::Version(0, 0, 0));
        chihaya::internal_misc::validate_missing_placeholder(nhandle, ritsuko::Version(1, 0, 0));

        auto shandle = fhandle.openDataSet("stringy");
        chihaya::internal_misc::validate_missing_placeholder(shandle, ritsuko::Version(1, 0, 0));
        chihaya::internal_misc::validate_missing_placeholder(shandle, ritsuko::Version(1, 1, 0));

        auto ihandle = fhandle.openDataSet("inty");
        chihaya::internal_misc::validate_missing_placeholder(ihandle, ritsuko::Version(1, 0, 0));
        expect_error([&]() { chihaya::internal_misc::validate_missing_placeholder(ihandle, ritsuko::Version(1, 1, 0)); }, "same type as");

        auto ihandle2 = fhandle.openDataSet("inty2");
        chihaya::internal_misc::validate_missing_placeholder(ihandle2, ritsuko::Version(1, 0, 0));
        chihaya::internal_misc::validate_missing_placeholder(ihandle2, ritsuko::Version(1, 1, 0));
    }
}

TEST(UtilsMisc, LoadAlong) {
    const char * path = "Test_utils_misc.h5";

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        add_numeric_vector<int>(fhandle, "along", { 1 }, H5::PredType::NATIVE_INT);
    }
    expect_error([&]() { chihaya::internal_misc::load_along(H5::H5File(path, H5F_ACC_RDONLY), ritsuko::Version(1, 1, 0)); }, "scalar");

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        add_numeric_scalar<int>(fhandle, "along", 1, H5::PredType::NATIVE_DOUBLE);
    }
    {
        H5::H5File fhandle(path, H5F_ACC_RDONLY);
        expect_error([&]() { chihaya::internal_misc::load_along(fhandle, ritsuko::Version(1, 0, 0)); }, "integer");
        expect_error([&]() { chihaya::internal_misc::load_along(fhandle, ritsuko::Version(1, 1, 0)); }, "64-bit unsigned");
    }

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        add_numeric_scalar<int>(fhandle, "along", -1, H5::PredType::NATIVE_INT);
    }
    expect_error([&]() { chihaya::internal_misc::load_along(H5::H5File(path, H5F_ACC_RDONLY), ritsuko::Version(1, 0, 0)); }, "non-negative");

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        add_numeric_scalar<int>(fhandle, "along", 1, H5::PredType::NATIVE_UINT8);
    }
    {
        H5::H5File fhandle(path, H5F_ACC_RDONLY);
        EXPECT_EQ(chihaya::internal_misc::load_along(fhandle, ritsuko::Version(1, 0, 0)), 1);
        EXPECT_EQ(chihaya::internal_misc::load_along(fhandle, ritsuko::Version(1, 1, 0)), 1);
    }
}

TEST(UtilsMisc, LoadSeedDetails) {
    const char * path = "Test_utils_misc.h5";
    chihaya::Options options;

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        fhandle.createGroup("seed");
    }
    expect_error([&]() { chihaya::internal_misc::load_seed_details(H5::H5File(path, H5F_ACC_RDONLY), "seed", ritsuko::Version(1, 1, 0), options); }, "failed to validate");

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        mock_array_opener(fhandle, "seed", { 13, 14 }, 1000000, "INTEGER");
    }
    auto deets = chihaya::internal_misc::load_seed_details(H5::H5File(path, H5F_ACC_RDONLY), "seed", ritsuko::Version(1, 0, 0), options); 
    EXPECT_EQ(deets.type, chihaya::INTEGER);
    EXPECT_EQ(deets.dimensions[0], 13);
    EXPECT_EQ(deets.dimensions[1], 14);
}

TEST(UtilsMisc, LoadScalarStringDataset) {
    const char * path = "Test_utils_misc.h5";

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        add_string_scalar(fhandle, "foo", "bar");
        add_string_vector(fhandle, "whee", 20);
        add_numeric_scalar<int>(fhandle, "stuff", 20, H5::PredType::NATIVE_INT);
    }
    {
        H5::H5File fhandle(path, H5F_ACC_RDONLY);
        expect_error([&]() { chihaya::internal_misc::load_scalar_string_dataset(fhandle, "lost"); }, "expected a dataset at 'lost'");
        expect_error([&]() { chihaya::internal_misc::load_scalar_string_dataset(fhandle, "whee"); }, "scalar");
        expect_error([&]() { chihaya::internal_misc::load_scalar_string_dataset(fhandle, "stuff"); }, "string");
        EXPECT_EQ(chihaya::internal_misc::load_scalar_string_dataset(fhandle, "foo"), "bar");
    }
}
