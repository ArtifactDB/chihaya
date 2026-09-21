#include <gtest/gtest.h>

#include <vector>
#include <cstddef>

#include "H5Cpp.h"
#include "ritsuko/ritsuko.hpp"
#include "chihaya/utils_list.hpp"

#include "utils.h"

class ValidateListTest : public ::testing::TestWithParam<ritsuko::Version> {};

TEST_P(ValidateListTest, Empty) {
    auto path = define_test_path("utils_list");
    auto version = GetParam();

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto lhandle = list_opener(fhandle, "x52", 4, version);
    }

    H5::H5File fhandle(path, H5F_ACC_RDONLY);
    auto ghandle = fhandle.openGroup("x52");
    auto deets = chihaya::validate_list(ghandle, version);
    EXPECT_EQ(deets.length, 4);
    EXPECT_EQ(deets.present.size(), 0);
}

TEST_P(ValidateListTest, NonEmpty) {
    auto path = define_test_path("utils_list");
    auto version = GetParam();

    // Partial occupancy.
    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto lhandle = list_opener(fhandle, "x52", 4, version);
        lhandle.createGroup("0");
        lhandle.createGroup("3");
    }
    {
        H5::H5File fhandle(path, H5F_ACC_RDONLY);
        auto ghandle = fhandle.openGroup("x52");
        auto deets = chihaya::validate_list(ghandle, version);
        EXPECT_EQ(deets.length, 4);
        EXPECT_EQ(deets.present.size(), 2);

        ASSERT_TRUE(deets.present.find(0) != deets.present.end());
        EXPECT_EQ(deets.present[0], "0");
        ASSERT_TRUE(deets.present.find(3) != deets.present.end());
        EXPECT_EQ(deets.present[3], "3");
    }

    // Full occupancy.
    {
        H5::H5File fhandle(path, H5F_ACC_RDWR);
        auto lhandle = fhandle.openGroup("x52");
        lhandle.createGroup("1");
        lhandle.createGroup("2");
    }
    {
        H5::H5File fhandle(path, H5F_ACC_RDONLY);
        auto ghandle = fhandle.openGroup("x52");
        auto deets = chihaya::validate_list(ghandle, version);
        EXPECT_EQ(deets.length, 4);
        EXPECT_EQ(deets.present.size(), 4);

        for (size_t i = 0; i < 4; ++i) {
            EXPECT_TRUE(deets.present.find(i) != deets.present.end());
            EXPECT_EQ(deets.present[i], std::to_string(i));
        }
    }
}

TEST_P(ValidateListTest, DoubleDigits) {
    auto path = define_test_path("utils_list");
    auto version = GetParam();

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto lhandle = list_opener(fhandle, "x52", 200, version);
        lhandle.createGroup("9");
        lhandle.createGroup("11");
        lhandle.createGroup("164");
    }

    H5::H5File fhandle(path, H5F_ACC_RDONLY);
    auto ghandle = fhandle.openGroup("x52");
    auto deets = chihaya::validate_list(ghandle, version);
    EXPECT_EQ(deets.length, 200);
    EXPECT_EQ(deets.present.size(), 3);

    ASSERT_TRUE(deets.present.find(9) != deets.present.end());
    EXPECT_EQ(deets.present[9], "9");
    ASSERT_TRUE(deets.present.find(11) != deets.present.end());
    EXPECT_EQ(deets.present[11], "11");
    ASSERT_TRUE(deets.present.find(164) != deets.present.end());
    EXPECT_EQ(deets.present[164], "164");
}

TEST_P(ValidateListTest, TypeError) {
    auto path = define_test_path("utils_list");
    auto version = GetParam();

    if (version.ge(1, 1, 0)) {
        return;
    }

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        operation_opener(fhandle, "foo", "whee");
    }

    expect_error([&]() -> void { 
        H5::H5File fhandle(path, H5F_ACC_RDONLY);
        chihaya::validate_list(fhandle.openGroup("foo"), version);
    }, "delayed_type = \"list\"");
}

TEST_P(ValidateListTest, LengthError) {
    auto path = define_test_path("utils_list");
    auto version = GetParam();

    if (version.lt(1, 1, 0)) {
        {
            H5::H5File fhandle(path, H5F_ACC_TRUNC);
            auto lhandle = list_opener(fhandle, "foo", 1, version);
            lhandle.removeAttr("delayed_length");
            hsize_t dims = 9;
            lhandle.createAttribute("delayed_length", H5::PredType::NATIVE_INT, H5::DataSpace(1, &dims));
        }
        expect_error([&]() -> void { 
            H5::H5File fhandle(path, H5F_ACC_RDONLY);
            chihaya::validate_list(fhandle.openGroup("foo"), version);
        }, "scalar");

        {
            H5::H5File fhandle(path, H5F_ACC_TRUNC);
            auto lhandle = list_opener(fhandle, "foo", 1, version);
            lhandle.removeAttr("delayed_length");
            add_string_attribute(lhandle, "delayed_length", "FOO");
        }
        expect_error([&]() -> void { 
            H5::H5File fhandle(path, H5F_ACC_RDONLY);
            chihaya::validate_list(fhandle.openGroup("foo"), version);
        }, "integer");

        {
            H5::H5File fhandle(path, H5F_ACC_TRUNC);
            auto lhandle = list_opener(fhandle, "foo", 1, version);
            lhandle.removeAttr("delayed_length");
            auto ahandle = lhandle.createAttribute("delayed_length", H5::PredType::NATIVE_INT, H5S_SCALAR);
            int val = -1;
            ahandle.write(H5::PredType::NATIVE_INT, &val);
        }
        expect_error([&]() -> void { 
            H5::H5File fhandle(path, H5F_ACC_RDONLY);
            chihaya::validate_list(fhandle.openGroup("foo"), version);
        }, "non-negative");

    } else {
        {
            H5::H5File fhandle(path, H5F_ACC_TRUNC);
            auto lhandle = list_opener(fhandle, "foo", 1, version);
            lhandle.removeAttr("length");
            constexpr hsize_t one = 1;
            H5::DataSpace lspace(1, &one); 
            lhandle.createAttribute("length", H5::PredType::NATIVE_UINT32, lspace);
        }
        expect_error([&]() -> void { 
            H5::H5File fhandle(path, H5F_ACC_RDONLY);
            chihaya::validate_list(fhandle.openGroup("foo"), version);
        }, "scalar");

        {
            H5::H5File fhandle(path, H5F_ACC_TRUNC);
            auto lhandle = list_opener(fhandle, "foo", 1, version);
            lhandle.removeAttr("length");
            lhandle.createAttribute("length", H5::PredType::NATIVE_FLOAT, H5S_SCALAR);
        }
        expect_error([&]() -> void { 
            H5::H5File fhandle(path, H5F_ACC_RDONLY);
            chihaya::validate_list(fhandle.openGroup("foo"), version);
        }, "64-bit unsigned integer");
    }
}

TEST_P(ValidateListTest, ContentError) {
    auto path = define_test_path("utils_list");
    auto version = GetParam();

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto lhandle = list_opener(fhandle, "foo", 1, version);
        lhandle.createGroup("0");
        lhandle.createGroup("1");
    }
    expect_error([&]() -> void { 
        H5::H5File fhandle(path, H5F_ACC_RDONLY);
        chihaya::validate_list(fhandle.openGroup("foo"), version);
    }, "more objects");
}

TEST_P(ValidateListTest, NameError) {
    auto path = define_test_path("utils_list");
    auto version = GetParam();

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto lhandle = list_opener(fhandle, "foo", 1, version);
        lhandle.createGroup("blah");
    }
    expect_error([&]() -> void { 
        H5::H5File fhandle(path, H5F_ACC_RDONLY);
        chihaya::validate_list(fhandle.openGroup("foo"), version);
    }, "not a valid");

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto lhandle = list_opener(fhandle, "foo", 1, version);
        lhandle.createGroup("2");
    }
    expect_error([&]() -> void { 
        H5::H5File fhandle(path, H5F_ACC_RDONLY);
        chihaya::validate_list(fhandle.openGroup("foo"), version);
    }, "out of range");
}

INSTANTIATE_TEST_SUITE_P(
    ValidateList,
    ValidateListTest,
    spawn_all_versions()
);
