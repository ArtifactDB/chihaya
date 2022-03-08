#include <gtest/gtest.h>
#include "chihaya/chihaya.hpp"
#include "utils.h"

TEST(List, Basic) {
    std::string path = "test_list.h5";

    // Mocking up a file.
    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto lhandle = list_opener(fhandle, "x52", 4);
    }
    {
        H5::H5File fhandle(path, H5F_ACC_RDONLY);
        auto ghandle = fhandle.openGroup("x52");
        auto deets = chihaya::validate_list(ghandle);
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
        auto deets = chihaya::validate_list(ghandle);
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
        auto deets = chihaya::validate_list(ghandle);
        EXPECT_EQ(deets.length, 4);
        EXPECT_EQ(deets.present.size(), 4);

        for (size_t i = 0; i < 4; ++i) {
            EXPECT_TRUE(deets.present.find(i) != deets.present.end());
            EXPECT_EQ(deets.present[i], std::to_string(i));
        }
    }
}

TEST(List, Errors) {
    std::string path = "test_list.h5";

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto lhandle = fhandle.createGroup("x52");
    }
    expect_error([&]() -> void { 
        H5::H5File fhandle(path, H5F_ACC_RDONLY);
        chihaya::validate_list(fhandle.openGroup("x52"));
    }, "delayed_type");

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        operation_opener(fhandle, "foo", "whee");
    }
    expect_error([&]() -> void { 
        H5::H5File fhandle(path, H5F_ACC_RDONLY);
        chihaya::validate_list(fhandle.openGroup("foo"));
    }, "delayed_type = \"list\"");

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto lhandle = list_opener(fhandle, "foo", 1);
        lhandle.removeAttr("delayed_length");
    }
    expect_error([&]() -> void { 
        H5::H5File fhandle(path, H5F_ACC_RDONLY);
        chihaya::validate_list(fhandle.openGroup("foo"));
    }, "delayed_length");

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto lhandle = list_opener(fhandle, "foo", 1);
        lhandle.removeAttr("delayed_length");
        lhandle.createAttribute("delayed_length", H5::PredType::NATIVE_FLOAT, H5S_SCALAR);
    }
    expect_error([&]() -> void { 
        H5::H5File fhandle(path, H5F_ACC_RDONLY);
        chihaya::validate_list(fhandle.openGroup("foo"));
    }, "delayed_length");

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto lhandle = list_opener(fhandle, "foo", 1);
        lhandle.createGroup("0");
        lhandle.createGroup("1");
    }
    expect_error([&]() -> void { 
        H5::H5File fhandle(path, H5F_ACC_RDONLY);
        chihaya::validate_list(fhandle.openGroup("foo"));
    }, "more objects");

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto lhandle = list_opener(fhandle, "foo", 1);
        lhandle.createGroup("blah");
    }
    expect_error([&]() -> void { 
        H5::H5File fhandle(path, H5F_ACC_RDONLY);
        chihaya::validate_list(fhandle.openGroup("foo"));
    }, "not a valid");

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto lhandle = list_opener(fhandle, "foo", 1);
        lhandle.createGroup("2");
    }
    expect_error([&]() -> void { 
        H5::H5File fhandle(path, H5F_ACC_RDONLY);
        chihaya::validate_list(fhandle.openGroup("foo"));
    }, "out of range");
}
