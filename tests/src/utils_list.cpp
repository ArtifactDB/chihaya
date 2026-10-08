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

        std::sort(deets.present.begin(), deets.present.end());
        EXPECT_EQ(deets.present[0], (std::pair<std::size_t, std::string>(0, "0")));
        EXPECT_EQ(deets.present[1], (std::pair<std::size_t, std::string>(3, "3")));
    }

    // Full occupancy.
    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto lhandle = list_opener(fhandle, "x52", 4, version);
        lhandle.createGroup("1");
        lhandle.createGroup("0");
        lhandle.createGroup("3");
        lhandle.createGroup("2");
    }
    {
        H5::H5File fhandle(path, H5F_ACC_RDONLY);
        auto ghandle = fhandle.openGroup("x52");
        auto deets = chihaya::validate_list(ghandle, version);
        EXPECT_EQ(deets.length, 4);
        EXPECT_EQ(deets.present.size(), 4);

        std::sort(deets.present.begin(), deets.present.end());
        for (size_t i = 0; i < 4; ++i) {
            EXPECT_EQ(deets.present[i], (std::pair<std::size_t, std::string>(i, std::to_string(i))));
        }
    }
}

TEST_P(ValidateListTest, ManyDigits) {
    auto path = define_test_path("utils_list");
    auto version = GetParam();

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto lhandle = list_opener(fhandle, "x52", 200, version);
        lhandle.createGroup("9");
        lhandle.createGroup("11");
        lhandle.createGroup("164");
    }
    {
        H5::H5File fhandle(path, H5F_ACC_RDONLY);
        auto ghandle = fhandle.openGroup("x52");
        auto deets = chihaya::validate_list(ghandle, version);
        EXPECT_EQ(deets.length, 200);
        EXPECT_EQ(deets.present.size(), 3);

        std::sort(deets.present.begin(), deets.present.end());
        EXPECT_EQ(deets.present[0], (std::pair<std::size_t, std::string>(9, "9")));
        EXPECT_EQ(deets.present[1], (std::pair<std::size_t, std::string>(11, "11")));
        EXPECT_EQ(deets.present[2], (std::pair<std::size_t, std::string>(164, "164")));
    }

    // As Kylo says, more.
    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto lhandle = list_opener(fhandle, "x52", 2345, version);
        lhandle.createGroup("2344");
        lhandle.createGroup("987");
        lhandle.createGroup("1357");
        lhandle.createGroup("0");
    }
    {
        H5::H5File fhandle(path, H5F_ACC_RDONLY);
        auto ghandle = fhandle.openGroup("x52");
        auto deets = chihaya::validate_list(ghandle, version);
        EXPECT_EQ(deets.length, 2345);
        EXPECT_EQ(deets.present.size(), 4);

        std::sort(deets.present.begin(), deets.present.end());
        EXPECT_EQ(deets.present[0], (std::pair<std::size_t, std::string>(0, "0")));
        EXPECT_EQ(deets.present[1], (std::pair<std::size_t, std::string>(987, "987")));
        EXPECT_EQ(deets.present[2], (std::pair<std::size_t, std::string>(1357, "1357")));
        EXPECT_EQ(deets.present[3], (std::pair<std::size_t, std::string>(2344, "2344")));
    }
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
        lhandle.createGroup("0001");
    }
    expect_error([&]() -> void { 
        H5::H5File fhandle(path, H5F_ACC_RDONLY);
        chihaya::validate_list(fhandle.openGroup("foo"), version);
    }, "leading zeros");

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto lhandle = list_opener(fhandle, "foo", 1, version);
        lhandle.createGroup("2");
    }
    expect_error([&]() -> void { 
        H5::H5File fhandle(path, H5F_ACC_RDONLY);
        chihaya::validate_list(fhandle.openGroup("foo"), version);
    }, "out of bounds");

    // Multi-digit out of bounds.
    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto lhandle = list_opener(fhandle, "foo", 1989, version);
        lhandle.createGroup("1989");
    }
    expect_error([&]() -> void { 
        H5::H5File fhandle(path, H5F_ACC_RDONLY);
        chihaya::validate_list(fhandle.openGroup("foo"), version);
    }, "out of bounds");

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto lhandle = list_opener(fhandle, "foo", 1989, version);
        lhandle.createGroup("2000");
    }
    expect_error([&]() -> void { 
        H5::H5File fhandle(path, H5F_ACC_RDONLY);
        chihaya::validate_list(fhandle.openGroup("foo"), version);
    }, "out of bounds");
}

INSTANTIATE_TEST_SUITE_P(
    ValidateList,
    ValidateListTest,
    spawn_all_versions()
);
