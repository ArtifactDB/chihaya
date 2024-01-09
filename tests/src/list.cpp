#include <gtest/gtest.h>
#include "chihaya/chihaya.hpp"
#include "utils.h"

class ListTest : public ::testing::TestWithParam<int> {
public:
    ListTest() : path("test_list.h5") {}
protected:
    std::string path;

    static ritsuko::Version convert_from_int(int version) {
        ritsuko::Version output;
        output.patch = version % 1000;
        version /= 1000;
        output.minor = version % 1000;
        version /= 1000;
        output.major = version;
        return output;
    }
};


TEST_P(ListTest, Basic) {
    auto raw_version = GetParam();
    auto version = convert_from_int(raw_version);

    // Mocking up a file.
    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto lhandle = list_opener(fhandle, "x52", 4, raw_version);
    }
    {
        H5::H5File fhandle(path, H5F_ACC_RDONLY);
        auto ghandle = fhandle.openGroup("x52");
        auto deets = chihaya::internal_list::validate(ghandle, version);
        EXPECT_EQ(deets.length, 4);
        EXPECT_EQ(deets.present.size(), 0);
    }

    // Loading the file (partial).
    {
        H5::H5File fhandle(path, H5F_ACC_RDWR);
        auto lhandle = fhandle.openGroup("x52");
        lhandle.createGroup("0");
        lhandle.createGroup("3");
    }
    {
        H5::H5File fhandle(path, H5F_ACC_RDONLY);
        auto ghandle = fhandle.openGroup("x52");
        auto deets = chihaya::internal_list::validate(ghandle, version);
        EXPECT_EQ(deets.length, 4);
        EXPECT_EQ(deets.present.size(), 2);

        EXPECT_TRUE(deets.present.find(0) != deets.present.end());
        EXPECT_EQ(deets.present[0], "0");
        EXPECT_TRUE(deets.present.find(3) != deets.present.end());
        EXPECT_EQ(deets.present[3], "3");
    }

    // Loading the file (partial).
    {
        H5::H5File fhandle(path, H5F_ACC_RDWR);
        auto lhandle = fhandle.openGroup("x52");
        lhandle.createGroup("1");
        lhandle.createGroup("2");
    }
    {
        H5::H5File fhandle(path, H5F_ACC_RDONLY);
        auto ghandle = fhandle.openGroup("x52");
        auto deets = chihaya::internal_list::validate(ghandle, version);
        EXPECT_EQ(deets.length, 4);
        EXPECT_EQ(deets.present.size(), 4);

        for (size_t i = 0; i < 4; ++i) {
            EXPECT_TRUE(deets.present.find(i) != deets.present.end());
            EXPECT_EQ(deets.present[i], std::to_string(i));
        }
    }
}

TEST_P(ListTest, Errors) {
    auto raw_version = GetParam();
    auto version = convert_from_int(raw_version);

    if (raw_version < 1100000) {
        {
            H5::H5File fhandle(path, H5F_ACC_TRUNC);
            auto lhandle = fhandle.createGroup("x52");
        }
        expect_error([&]() -> void { 
            H5::H5File fhandle(path, H5F_ACC_RDONLY);
            chihaya::internal_list::validate(fhandle.openGroup("x52"), version);
        }, "delayed_type");

        {
            H5::H5File fhandle(path, H5F_ACC_TRUNC);
            operation_opener(fhandle, "foo", "whee");
        }
        expect_error([&]() -> void { 
            H5::H5File fhandle(path, H5F_ACC_RDONLY);
            chihaya::internal_list::validate(fhandle.openGroup("foo"), version);
        }, "delayed_type = \"list\"");

        {
            H5::H5File fhandle(path, H5F_ACC_TRUNC);
            auto lhandle = list_opener(fhandle, "foo", 1, raw_version);
            lhandle.removeAttr("delayed_length");
        }
        expect_error([&]() -> void { 
            H5::H5File fhandle(path, H5F_ACC_RDONLY);
            chihaya::internal_list::validate(fhandle.openGroup("foo"), version);
        }, "delayed_length");

        {
            H5::H5File fhandle(path, H5F_ACC_TRUNC);
            auto lhandle = list_opener(fhandle, "foo", 1, raw_version);
            lhandle.removeAttr("delayed_length");
            lhandle.createAttribute("delayed_length", H5::PredType::NATIVE_FLOAT, H5S_SCALAR);
        }
        expect_error([&]() -> void { 
            H5::H5File fhandle(path, H5F_ACC_RDONLY);
            chihaya::internal_list::validate(fhandle.openGroup("foo"), version);
        }, "delayed_length");

    } else {
        {
            H5::H5File fhandle(path, H5F_ACC_TRUNC);
            auto lhandle = list_opener(fhandle, "foo", 1, raw_version);
            lhandle.removeAttr("length");
        }
        expect_error([&]() -> void { 
            H5::H5File fhandle(path, H5F_ACC_RDONLY);
            chihaya::internal_list::validate(fhandle.openGroup("foo"), version);
        }, "length");

        {
            H5::H5File fhandle(path, H5F_ACC_TRUNC);
            auto lhandle = list_opener(fhandle, "foo", 1, raw_version);
            lhandle.removeAttr("length");
            lhandle.createAttribute("length", H5::PredType::NATIVE_FLOAT, H5S_SCALAR);
        }
        expect_error([&]() -> void { 
            H5::H5File fhandle(path, H5F_ACC_RDONLY);
            chihaya::internal_list::validate(fhandle.openGroup("foo"), version);
        }, "length");
    }
}

TEST_P(ListTest, ElementErrors) {
    auto raw_version = GetParam();
    auto version = convert_from_int(raw_version);

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto lhandle = list_opener(fhandle, "foo", 1, raw_version);
        lhandle.createGroup("0");
        lhandle.createGroup("1");
    }
    expect_error([&]() -> void { 
        H5::H5File fhandle(path, H5F_ACC_RDONLY);
        chihaya::internal_list::validate(fhandle.openGroup("foo"), version);
    }, "more objects");

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto lhandle = list_opener(fhandle, "foo", 1, raw_version);
        lhandle.createGroup("blah");
    }
    expect_error([&]() -> void { 
        H5::H5File fhandle(path, H5F_ACC_RDONLY);
        chihaya::internal_list::validate(fhandle.openGroup("foo"), version);
    }, "not a valid");

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto lhandle = list_opener(fhandle, "foo", 1, raw_version);
        lhandle.createGroup("2");
    }
    expect_error([&]() -> void { 
        H5::H5File fhandle(path, H5F_ACC_RDONLY);
        chihaya::internal_list::validate(fhandle.openGroup("foo"), version);
    }, "out of range");
}

INSTANTIATE_TEST_SUITE_P(
    List,
    ListTest,
    ::testing::Values(0, 1000000, 1100000)
);
