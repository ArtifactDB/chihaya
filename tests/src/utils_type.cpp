#include <gtest/gtest.h>
#include "chihaya/utils_type.hpp"
#include "utils.h"

TEST(UtilsType, IsBoolean) {
    const char* path = "Test_utils_type.h5";

    {
        H5::H5File handle(path, H5F_ACC_TRUNC);
        auto dhandle = handle.createDataSet("foobar", H5::PredType::NATIVE_INT, H5S_SCALAR);
        EXPECT_FALSE(chihaya::internal_type::is_boolean(dhandle));
    }

    {
        H5::H5File handle(path, H5F_ACC_RDWR);
        auto dhandle = handle.openDataSet("foobar");
        add_string_attribute(dhandle, "is_boolean", "stuff");
    }
    {
        H5::H5File handle(path, H5F_ACC_RDONLY);
        auto dhandle = handle.openDataSet("foobar");
        expect_error([&]() { chihaya::internal_type::is_boolean(dhandle); }, "should be integer");
    }

    {
        H5::H5File handle(path, H5F_ACC_RDWR);
        auto dhandle = handle.openDataSet("foobar");
        dhandle.removeAttr("is_boolean");

        hsize_t dims[1] = { 9 };
        H5::DataSpace dspace(1, dims);
        dhandle.createAttribute("is_boolean", H5::PredType::NATIVE_INT32, dspace);
    }
    {
        H5::H5File handle(path, H5F_ACC_RDONLY);
        auto dhandle = handle.openDataSet("foobar");
        expect_error([&]() { chihaya::internal_type::is_boolean(dhandle); }, "should be a scalar");
    }

    {
        H5::H5File handle(path, H5F_ACC_TRUNC);
        auto dhandle = handle.createDataSet("foobar", H5::PredType::NATIVE_DOUBLE, H5S_SCALAR);
        dhandle.createAttribute("is_boolean", H5::PredType::NATIVE_INT, H5S_SCALAR);
    }
    {
        H5::H5File handle(path, H5F_ACC_RDONLY);
        auto dhandle = handle.openDataSet("foobar");
        expect_error([&]() { chihaya::internal_type::is_boolean(dhandle); }, "should only exist for integer datasets");
    }
}

TEST(UtilsType, TranslateType) {
    // New.
    EXPECT_EQ(chihaya::internal_type::translate_type_1_1("INTEGER"), chihaya::INTEGER);
    EXPECT_EQ(chihaya::internal_type::translate_type_1_1("FLOAT"), chihaya::FLOAT);
    EXPECT_EQ(chihaya::internal_type::translate_type_1_1("BOOLEAN"), chihaya::BOOLEAN);
    EXPECT_EQ(chihaya::internal_type::translate_type_1_1("STRING"), chihaya::STRING);
    expect_error([&]() { chihaya::internal_type::translate_type_1_1("FOO"); }, "unknown type");

    // Old.
    EXPECT_EQ(chihaya::internal_type::translate_type_0_0(H5T_INTEGER), chihaya::INTEGER);
    EXPECT_EQ(chihaya::internal_type::translate_type_0_0(H5T_FLOAT), chihaya::FLOAT);
    EXPECT_EQ(chihaya::internal_type::translate_type_0_0(H5T_STRING), chihaya::STRING);
    expect_error([&]() { chihaya::internal_type::translate_type_0_0(H5T_TIME); }, "unsupported");
}

TEST(UtilsType, CheckType) {
    const char* path = "Test_utils_type.h5";

    {
        H5::H5File handle(path, H5F_ACC_TRUNC);
        add_string_vector(handle, "stringy", 20);
        add_numeric_vector<int>(handle, "i8", {1,2,3}, H5::PredType::NATIVE_INT8);
        add_numeric_vector<int>(handle, "u16", {1,2,3}, H5::PredType::NATIVE_UINT16);
        add_numeric_vector<int>(handle, "i32", {1,2,3}, H5::PredType::NATIVE_INT32);
        add_numeric_vector<int>(handle, "u32", {1,2,3}, H5::PredType::NATIVE_UINT32);
        add_numeric_vector<int>(handle, "flt", {1,2,3}, H5::PredType::NATIVE_FLOAT);
        add_numeric_vector<int>(handle, "dbl", {1,2,3}, H5::PredType::NATIVE_DOUBLE);
    }

    H5::H5File handle(path, H5F_ACC_RDONLY);
    expect_error([&]() { chihaya::internal_type::check_type_1_1(handle.openDataSet("stringy"), chihaya::INTEGER); }, "32-bit signed integer");
    chihaya::internal_type::check_type_1_1(handle.openDataSet("i8"), chihaya::INTEGER); 
    chihaya::internal_type::check_type_1_1(handle.openDataSet("u16"), chihaya::INTEGER); 
    chihaya::internal_type::check_type_1_1(handle.openDataSet("i32"), chihaya::INTEGER); 
    expect_error([&]() { chihaya::internal_type::check_type_1_1(handle.openDataSet("u32"), chihaya::INTEGER); }, "32-bit signed integer");
    expect_error([&]() { chihaya::internal_type::check_type_1_1(handle.openDataSet("flt"), chihaya::INTEGER); }, "32-bit signed integer");
    expect_error([&]() { chihaya::internal_type::check_type_1_1(handle.openDataSet("dbl"), chihaya::INTEGER); }, "32-bit signed integer");

    expect_error([&]() { chihaya::internal_type::check_type_1_1(handle.openDataSet("stringy"), chihaya::BOOLEAN); }, "8-bit signed integer");
    chihaya::internal_type::check_type_1_1(handle.openDataSet("i8"), chihaya::BOOLEAN); 
    expect_error([&]() { chihaya::internal_type::check_type_1_1(handle.openDataSet("u16"), chihaya::BOOLEAN); }, "8-bit signed integer");
    expect_error([&]() { chihaya::internal_type::check_type_1_1(handle.openDataSet("i32"), chihaya::BOOLEAN); }, "8-bit signed integer");
    expect_error([&]() { chihaya::internal_type::check_type_1_1(handle.openDataSet("flt"), chihaya::BOOLEAN); }, "8-bit signed integer");
    expect_error([&]() { chihaya::internal_type::check_type_1_1(handle.openDataSet("dbl"), chihaya::BOOLEAN); }, "8-bit signed integer");

    expect_error([&]() { chihaya::internal_type::check_type_1_1(handle.openDataSet("stringy"), chihaya::FLOAT); }, "64-bit float");
    chihaya::internal_type::check_type_1_1(handle.openDataSet("i8"), chihaya::FLOAT); 
    chihaya::internal_type::check_type_1_1(handle.openDataSet("u16"), chihaya::FLOAT);
    chihaya::internal_type::check_type_1_1(handle.openDataSet("i32"), chihaya::FLOAT);
    chihaya::internal_type::check_type_1_1(handle.openDataSet("u32"), chihaya::FLOAT);
    chihaya::internal_type::check_type_1_1(handle.openDataSet("flt"), chihaya::FLOAT);
    chihaya::internal_type::check_type_1_1(handle.openDataSet("dbl"), chihaya::FLOAT);

    chihaya::internal_type::check_type_1_1(handle.openDataSet("stringy"), chihaya::STRING); 
    expect_error([&]() { chihaya::internal_type::check_type_1_1(handle.openDataSet("i8"), chihaya::STRING); }, "UTF-8 encoded string");
    expect_error([&]() { chihaya::internal_type::check_type_1_1(handle.openDataSet("u16"), chihaya::STRING); }, "UTF-8 encoded string");
    expect_error([&]() { chihaya::internal_type::check_type_1_1(handle.openDataSet("i32"), chihaya::STRING); }, "UTF-8 encoded string");
    expect_error([&]() { chihaya::internal_type::check_type_1_1(handle.openDataSet("u32"), chihaya::STRING); }, "UTF-8 encoded string");
    expect_error([&]() { chihaya::internal_type::check_type_1_1(handle.openDataSet("flt"), chihaya::STRING); }, "UTF-8 encoded string");
    expect_error([&]() { chihaya::internal_type::check_type_1_1(handle.openDataSet("dbl"), chihaya::STRING); }, "UTF-8 encoded string");
}
