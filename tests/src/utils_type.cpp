#include <gtest/gtest.h>

#include "H5Cpp.h"
#include "chihaya/utils_type.hpp"

#include "utils.h"

TEST(LoadNonNegativeIntegerScalar_0_99, Dataset) {
    auto path = define_test_path("utils_type");

    // Signed.
    {
        H5::H5File handle(path, H5F_ACC_TRUNC);
        add_numeric_scalar<int>(handle, "foo", 93, H5::PredType::NATIVE_INT32);
    }
    {
        H5::H5File handle(path, H5F_ACC_RDONLY);
        EXPECT_EQ(chihaya::load_non_negative_integer_scalar_0_99<int>(handle.openDataSet("foo")), 93);
    }

    {
        H5::H5File handle(path, H5F_ACC_TRUNC);
        add_numeric_scalar<int>(handle, "foo", -10, H5::PredType::NATIVE_INT32);
    }
    {
        H5::H5File handle(path, H5F_ACC_RDONLY);
        expect_error([&]() -> void {
            chihaya::load_non_negative_integer_scalar_0_99<int>(handle.openDataSet("foo"));
        }, "non-negative");
    }

    // Unsigned.
    {
        H5::H5File handle(path, H5F_ACC_TRUNC);
        add_numeric_scalar<int>(handle, "foo", 1000, H5::PredType::NATIVE_UINT64);
    }
    {
        H5::H5File handle(path, H5F_ACC_RDONLY);
        EXPECT_EQ(chihaya::load_non_negative_integer_scalar_0_99<int>(handle.openDataSet("foo")), 1000);
    }
}

TEST(LoadNonNegativeIntegerScalar_0_99, Attribute) {
    auto path = define_test_path("utils_type");

    // Signed.
    {
        H5::H5File handle(path, H5F_ACC_TRUNC);
        auto ghandle = handle.createGroup("foo");
        auto ahandle = ghandle.createAttribute("bar", H5::PredType::NATIVE_INT64, H5S_SCALAR);
        const int val = 798;
        ahandle.write(H5::PredType::NATIVE_INT, &val);
    }
    {
        H5::H5File handle(path, H5F_ACC_RDONLY);
        auto ghandle = handle.openGroup("foo");
        EXPECT_EQ(chihaya::load_non_negative_integer_scalar_0_99<int>(ghandle.openAttribute("bar")), 798);
    }

    {
        H5::H5File handle(path, H5F_ACC_TRUNC);
        auto ghandle = handle.createGroup("foo");
        auto ahandle = ghandle.createAttribute("bar", H5::PredType::NATIVE_INT64, H5S_SCALAR);
        const int val = -10;
        ahandle.write(H5::PredType::NATIVE_INT, &val);
    }
    {
        H5::H5File handle(path, H5F_ACC_RDONLY);
        auto ghandle = handle.openGroup("foo");
        expect_error([&]() -> void {
            chihaya::load_non_negative_integer_scalar_0_99<int>(ghandle.openAttribute("bar"));
        }, "non-negative");
    }

    // Unsigned.
    {
        H5::H5File handle(path, H5F_ACC_TRUNC);
        auto ghandle = handle.createGroup("foo");
        auto ahandle = ghandle.createAttribute("bar", H5::PredType::NATIVE_UINT16, H5S_SCALAR);
        const int val = 12398;
        ahandle.write(H5::PredType::NATIVE_INT, &val);
    }
    {
        H5::H5File handle(path, H5F_ACC_RDONLY);
        auto ghandle = handle.openGroup("foo");
        EXPECT_EQ(chihaya::load_non_negative_integer_scalar_0_99<int>(ghandle.openAttribute("bar")), 12398);
    }
}

TEST(LoadNonNegativeIntegerVector_0_99, Basic) {
    auto path = define_test_path("utils_type");

    // Signed.
    std::vector<std::size_t> expected{ 93, 231 };
    {
        H5::H5File handle(path, H5F_ACC_TRUNC);
        add_numeric_vector(handle, "foo", expected, H5::PredType::NATIVE_INT32);
    }
    {
        H5::H5File handle(path, H5F_ACC_RDONLY);
        EXPECT_EQ(chihaya::load_non_negative_integer_vector_0_99<std::size_t>(handle.openDataSet("foo"), expected.size()), expected);
    }

    {
        H5::H5File handle(path, H5F_ACC_TRUNC);
        add_numeric_vector<int>(handle, "foo", { -10, 23 }, H5::PredType::NATIVE_INT32);
    }
    {
        H5::H5File handle(path, H5F_ACC_RDONLY);
        expect_error([&]() -> void {
            chihaya::load_non_negative_integer_vector_0_99<std::size_t>(handle.openDataSet("foo"), 2);
        }, "non-negative");
    }

    // Unsigned.
    std::vector<std::uint8_t> expected2{ 199, 238, 3, 231 };
    {
        H5::H5File handle(path, H5F_ACC_TRUNC);
        add_numeric_vector(handle, "foo", expected2, H5::PredType::NATIVE_UINT64);
    }
    {
        H5::H5File handle(path, H5F_ACC_RDONLY);
        EXPECT_EQ(chihaya::load_non_negative_integer_vector_0_99<std::uint8_t>(handle.openDataSet("foo"), expected2.size()), expected2);
    }
}

TEST(LoadBooleanScalar_0_99, Dataset) {
    auto path = define_test_path("utils_type");

    // Signed.
    {
        H5::H5File handle(path, H5F_ACC_TRUNC);
        add_numeric_scalar<int>(handle, "foo", 93, H5::PredType::NATIVE_INT32);
        add_numeric_scalar<int>(handle, "negfoo", -10, H5::PredType::NATIVE_INT32);
        add_numeric_scalar<int>(handle, "bar", 0, H5::PredType::NATIVE_INT32);
    }
    {
        H5::H5File handle(path, H5F_ACC_RDONLY);
        EXPECT_TRUE(chihaya::load_boolean_scalar_0_99(handle.openDataSet("foo")));
        EXPECT_TRUE(chihaya::load_boolean_scalar_0_99(handle.openDataSet("negfoo")));
        EXPECT_FALSE(chihaya::load_boolean_scalar_0_99(handle.openDataSet("bar")));
    }

    // Unsigned.
    {
        H5::H5File handle(path, H5F_ACC_TRUNC);
        add_numeric_scalar<int>(handle, "foo", 1000, H5::PredType::NATIVE_UINT64);
        add_numeric_scalar<int>(handle, "bar", 0, H5::PredType::NATIVE_UINT16);
    }
    {
        H5::H5File handle(path, H5F_ACC_RDONLY);
        EXPECT_TRUE(chihaya::load_boolean_scalar_0_99(handle.openDataSet("foo")));
        EXPECT_FALSE(chihaya::load_boolean_scalar_0_99(handle.openDataSet("bar")));
    }
}

TEST(LoadBooleanScalar_0_99, Attribute) {
    auto path = define_test_path("utils_type");

    // Signed.
    {
        H5::H5File handle(path, H5F_ACC_TRUNC);
        auto ghandle = handle.createGroup("stuff");
        { 
            auto ahandle = ghandle.createAttribute("foo", H5::PredType::NATIVE_INT16, H5S_SCALAR);
            const int val = 798;
            ahandle.write(H5::PredType::NATIVE_INT, &val);
        }
        { 
            auto ahandle = ghandle.createAttribute("negfoo", H5::PredType::NATIVE_INT32, H5S_SCALAR);
            const int val = -123;
            ahandle.write(H5::PredType::NATIVE_INT, &val);
        }
        {
            auto ahandle = ghandle.createAttribute("bar", H5::PredType::NATIVE_INT8, H5S_SCALAR);
            const int val = 0;
            ahandle.write(H5::PredType::NATIVE_INT, &val);
        }
    }
    {
        H5::H5File handle(path, H5F_ACC_RDONLY);
        auto ghandle = handle.openGroup("stuff");
        EXPECT_TRUE(chihaya::load_boolean_scalar_0_99(ghandle.openAttribute("foo")));
        EXPECT_TRUE(chihaya::load_boolean_scalar_0_99(ghandle.openAttribute("negfoo")));
        EXPECT_FALSE(chihaya::load_boolean_scalar_0_99(ghandle.openAttribute("bar"))); 
    }

    // Unsigned.
    {
        H5::H5File handle(path, H5F_ACC_TRUNC);
        auto ghandle = handle.createGroup("stuff");
        {
            auto ahandle = ghandle.createAttribute("foo", H5::PredType::NATIVE_UINT16, H5S_SCALAR);
            const int val = 12398;
            ahandle.write(H5::PredType::NATIVE_INT, &val);
        }
        {
            auto ahandle = ghandle.createAttribute("bar", H5::PredType::NATIVE_UINT64, H5S_SCALAR);
            const int val = 0;
            ahandle.write(H5::PredType::NATIVE_INT, &val);
        }
    }
    {
        H5::H5File handle(path, H5F_ACC_RDONLY);
        auto ghandle = handle.openGroup("stuff");
        EXPECT_TRUE(chihaya::load_boolean_scalar_0_99(ghandle.openAttribute("foo")));
        EXPECT_FALSE(chihaya::load_boolean_scalar_0_99(ghandle.openAttribute("bar")));
    }
}

TEST(IsBoolean_0_99, Okay) {
    auto path = define_test_path("utils_type");

    {
        H5::H5File handle(path, H5F_ACC_TRUNC);
        auto dhandle = handle.createDataSet("foobar", H5::PredType::NATIVE_INT, H5S_SCALAR);
        EXPECT_FALSE(chihaya::is_boolean_0_99(dhandle));
    }

    {
        H5::H5File handle(path, H5F_ACC_RDWR);
        auto dhandle = handle.openDataSet("foobar");
        auto ahandle = dhandle.createAttribute("is_boolean", H5::PredType::NATIVE_INT8, H5S_SCALAR);
        const int val = 0;
        ahandle.write(H5::PredType::NATIVE_INT, &val);
    }
    {
        H5::H5File handle(path, H5F_ACC_RDONLY);
        EXPECT_FALSE(chihaya::is_boolean_0_99(handle.openDataSet("foobar")));
    }

    {
        H5::H5File handle(path, H5F_ACC_RDWR);
        auto dhandle = handle.openDataSet("foobar");
        auto ahandle = dhandle.openAttribute("is_boolean");
        const int val = 1000;
        ahandle.write(H5::PredType::NATIVE_INT, &val);
    }
    {
        H5::H5File handle(path, H5F_ACC_RDONLY);
        EXPECT_TRUE(chihaya::is_boolean_0_99(handle.openDataSet("foobar")));
    }
}

TEST(IsBoolean_0_99, Error) {
    auto path = define_test_path("utils_type");

    // Test that we check for a numeric scalar 'is_boolean'.
    {
        H5::H5File handle(path, H5F_ACC_TRUNC);
        auto dhandle = handle.createDataSet("foobar", H5::PredType::NATIVE_INT, H5S_SCALAR);
        add_string_attribute(dhandle, "is_boolean", "stuff");
    }
    {
        H5::H5File handle(path, H5F_ACC_RDONLY);
        auto dhandle = handle.openDataSet("foobar");
        expect_error([&]() -> void {
            chihaya::is_boolean_0_99(dhandle);
        }, "expected an integer");
    }

    {
        H5::H5File handle(path, H5F_ACC_RDWR);
        auto dhandle = handle.openDataSet("foobar");
        dhandle.removeAttr("is_boolean");
        hsize_t dims = 9;
        dhandle.createAttribute("is_boolean", H5::PredType::NATIVE_INT32, H5::DataSpace(1, &dims));
    }
    {
        H5::H5File handle(path, H5F_ACC_RDONLY);
        auto dhandle = handle.openDataSet("foobar");
        expect_error([&]() -> void {
            chihaya::is_boolean_0_99(dhandle);
        }, "should be a scalar");
    }

    {
        H5::H5File handle(path, H5F_ACC_TRUNC);
        auto dhandle = handle.createDataSet("foobar", H5::PredType::NATIVE_DOUBLE, H5S_SCALAR);
        dhandle.createAttribute("is_boolean", H5::PredType::NATIVE_INT, H5S_SCALAR);
    }
    {
        H5::H5File handle(path, H5F_ACC_RDONLY);
        auto dhandle = handle.openDataSet("foobar");
        expect_error([&]() -> void {
            chihaya::is_boolean_0_99(dhandle);
        }, "should only exist for integer datasets");
    }
}

TEST(TranslateType, New) {
    EXPECT_EQ(chihaya::translate_type_1_1("INTEGER"), chihaya::INTEGER);
    EXPECT_EQ(chihaya::translate_type_1_1("FLOAT"), chihaya::FLOAT);
    EXPECT_EQ(chihaya::translate_type_1_1("BOOLEAN"), chihaya::BOOLEAN);
    EXPECT_EQ(chihaya::translate_type_1_1("STRING"), chihaya::STRING);

    expect_error([&]() -> void {
        chihaya::translate_type_1_1("FOO");
    }, "unknown type");
}

TEST(TranslateType, Old) {
    EXPECT_EQ(chihaya::translate_type_0_99(H5T_INTEGER), chihaya::INTEGER);
    EXPECT_EQ(chihaya::translate_type_0_99(H5T_FLOAT), chihaya::FLOAT);
    EXPECT_EQ(chihaya::translate_type_0_99(H5T_STRING), chihaya::STRING);

    expect_error([&]() -> void {
        chihaya::translate_type_0_99(H5T_TIME);
    }, "unsupported");
}

TEST(CheckType_1_1, Basic) {
    auto path = define_test_path("utils_type");

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
    expect_error([&]() { chihaya::check_type_1_1(handle.openDataSet("stringy"), chihaya::INTEGER); }, "32-bit signed integer");
    chihaya::check_type_1_1(handle.openDataSet("i8"), chihaya::INTEGER); 
    chihaya::check_type_1_1(handle.openDataSet("u16"), chihaya::INTEGER); 
    chihaya::check_type_1_1(handle.openDataSet("i32"), chihaya::INTEGER); 
    expect_error([&]() { chihaya::check_type_1_1(handle.openDataSet("u32"), chihaya::INTEGER); }, "32-bit signed integer");
    expect_error([&]() { chihaya::check_type_1_1(handle.openDataSet("flt"), chihaya::INTEGER); }, "32-bit signed integer");
    expect_error([&]() { chihaya::check_type_1_1(handle.openDataSet("dbl"), chihaya::INTEGER); }, "32-bit signed integer");

    expect_error([&]() { chihaya::check_type_1_1(handle.openDataSet("stringy"), chihaya::BOOLEAN); }, "8-bit signed integer");
    chihaya::check_type_1_1(handle.openDataSet("i8"), chihaya::BOOLEAN); 
    expect_error([&]() { chihaya::check_type_1_1(handle.openDataSet("u16"), chihaya::BOOLEAN); }, "8-bit signed integer");
    expect_error([&]() { chihaya::check_type_1_1(handle.openDataSet("i32"), chihaya::BOOLEAN); }, "8-bit signed integer");
    expect_error([&]() { chihaya::check_type_1_1(handle.openDataSet("flt"), chihaya::BOOLEAN); }, "8-bit signed integer");
    expect_error([&]() { chihaya::check_type_1_1(handle.openDataSet("dbl"), chihaya::BOOLEAN); }, "8-bit signed integer");

    expect_error([&]() { chihaya::check_type_1_1(handle.openDataSet("stringy"), chihaya::FLOAT); }, "64-bit float");
    chihaya::check_type_1_1(handle.openDataSet("i8"), chihaya::FLOAT); 
    chihaya::check_type_1_1(handle.openDataSet("u16"), chihaya::FLOAT);
    chihaya::check_type_1_1(handle.openDataSet("i32"), chihaya::FLOAT);
    chihaya::check_type_1_1(handle.openDataSet("u32"), chihaya::FLOAT);
    chihaya::check_type_1_1(handle.openDataSet("flt"), chihaya::FLOAT);
    chihaya::check_type_1_1(handle.openDataSet("dbl"), chihaya::FLOAT);

    chihaya::check_type_1_1(handle.openDataSet("stringy"), chihaya::STRING); 
    expect_error([&]() { chihaya::check_type_1_1(handle.openDataSet("i8"), chihaya::STRING); }, "UTF-8 encoded string");
    expect_error([&]() { chihaya::check_type_1_1(handle.openDataSet("u16"), chihaya::STRING); }, "UTF-8 encoded string");
    expect_error([&]() { chihaya::check_type_1_1(handle.openDataSet("i32"), chihaya::STRING); }, "UTF-8 encoded string");
    expect_error([&]() { chihaya::check_type_1_1(handle.openDataSet("u32"), chihaya::STRING); }, "UTF-8 encoded string");
    expect_error([&]() { chihaya::check_type_1_1(handle.openDataSet("flt"), chihaya::STRING); }, "UTF-8 encoded string");
    expect_error([&]() { chihaya::check_type_1_1(handle.openDataSet("dbl"), chihaya::STRING); }, "UTF-8 encoded string");
}
