#include <gtest/gtest.h>
#include "chihaya/chihaya.hpp"
#include "utils.h"

class UnaryArithmeticTest : public ::testing::TestWithParam<int> {
public:
    UnaryArithmeticTest() : path("Test_unary_arithmetic.h5") {}
protected:
    std::string path;

    static H5::Group unary_arithmetic_opener(H5::Group& handle, const std::string& name, const std::string& method, const std::string& side, const std::vector<int>& dimensions, int version, const std::string& type) {
        auto ghandle = operation_opener(handle, name, "unary arithmetic");
        add_version_string(ghandle, version);
        mock_array_opener(ghandle, "seed", dimensions, version, type);
        add_string_scalar(ghandle, "method", method);
        add_string_scalar(ghandle, "side", side);
        return ghandle;
    }
};

TEST_P(UnaryArithmeticTest, PureUnary) {
    auto version = GetParam();

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        unary_arithmetic_opener(fhandle, "hello", "+", "none", { 17, 21 }, version, "INTEGER");
    }

    auto output = chihaya::validate(path, "hello"); 
    EXPECT_EQ(output.type, chihaya::INTEGER);
    EXPECT_EQ(output.dimensions.size(), 2);
    EXPECT_EQ(output.dimensions[0], 17);
    EXPECT_EQ(output.dimensions[1], 21);
}

TEST_P(UnaryArithmeticTest, ScalarUnary) {
    auto version = GetParam();

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = unary_arithmetic_opener(fhandle, "hello", "-", "left", { 23, 11 }, version, "INTEGER");
        auto dhandle = add_numeric_scalar<double>(ghandle, "value", 2.5, H5::PredType::NATIVE_DOUBLE);
        if (version >= 1100000) {
            add_string_attribute(dhandle, "type", "FLOAT");
        }
    }
    {
        auto output = chihaya::validate(path, "hello"); 
        EXPECT_EQ(output.type, chihaya::FLOAT);
        EXPECT_EQ(output.dimensions.size(), 2);
        EXPECT_EQ(output.dimensions[0], 23);
        EXPECT_EQ(output.dimensions[1], 11);
    }

    // Boolean promotion works as expected.
    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = unary_arithmetic_opener(fhandle, "hello", "-", "left", { 23, 11 }, version, "BOOLEAN");
        auto dhandle = add_numeric_scalar<int>(ghandle, "value", 1, H5::PredType::NATIVE_INT8);
        if (version >= 1100000) {
            add_string_attribute(dhandle, "type", "BOOLEAN");
        }
    }
    {
        auto output = chihaya::validate(path, "hello"); 
        EXPECT_EQ(output.type, chihaya::INTEGER);
    }


    // Regular division always yields floats.
    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = unary_arithmetic_opener(fhandle, "hello", "/", "left", { 23, 11 }, version, "BOOLEAN");
        auto dhandle = add_numeric_scalar<int>(ghandle, "value", 2, H5::PredType::NATIVE_INT32);
        if (version >= 1100000) {
            add_string_attribute(dhandle, "type", "INTEGER");
        }
    }
    {
        auto output = chihaya::validate(path, "hello"); 
        EXPECT_EQ(output.type, chihaya::FLOAT);
    }

    // Integer division always yields integers.
    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = unary_arithmetic_opener(fhandle, "hello", "%/%", "left", { 23, 11 }, version, "FLOAT");
        auto dhandle = add_numeric_scalar<double>(ghandle, "value", 2.5, H5::PredType::NATIVE_DOUBLE);
        if (version >= 1100000) {
            add_string_attribute(dhandle, "type", "FLOAT");
        }
    }
    {
        auto output = chihaya::validate(path, "hello"); 
        EXPECT_EQ(output.type, chihaya::INTEGER);
    }
}

TEST_P(UnaryArithmeticTest, VectorUnary) {
    auto version = GetParam();

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = unary_arithmetic_opener(fhandle, "hello", "/", "left", { 5, 19 }, version, "INTEGER");
        auto dhandle = add_numeric_vector<int>(ghandle, "value", { 1, 2, 3, 4, 5 }, H5::PredType::NATIVE_INT16);
        if (version >= 1100000) {
            add_string_attribute(dhandle, "type", "INTEGER");
            add_numeric_scalar<int>(ghandle, "along", 0, H5::PredType::NATIVE_UINT8);
        } else {
            add_numeric_scalar<int>(ghandle, "along", 0, H5::PredType::NATIVE_INT);
        }
    }

    auto output = chihaya::validate(path, "hello"); 
    EXPECT_EQ(output.type, chihaya::FLOAT);
    EXPECT_EQ(output.dimensions.size(), 2);
    EXPECT_EQ(output.dimensions[0], 5);
    EXPECT_EQ(output.dimensions[1], 19);
}

TEST_P(UnaryArithmeticTest, Missing) {
    auto version = GetParam();
    if (version < 1000000) {
        return;
    }

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = unary_arithmetic_opener(fhandle, "hello", "*", "right", { 10, 7 }, version, "FLOAT");
        auto dhandle = add_numeric_vector<int>(ghandle, "value", { 6, 5, 4, 3, 2, 1, 0 }, H5::PredType::NATIVE_INT16);
        if (version >= 1100000) {
            add_string_attribute(dhandle, "type", "INTEGER");
            add_numeric_scalar<int>(ghandle, "along", 1, H5::PredType::NATIVE_UINT8);
        } else {
            add_numeric_scalar<int>(ghandle, "along", 1, H5::PredType::NATIVE_INT);
        }
        add_numeric_missing_placeholder<int>(dhandle, 2, H5::PredType::NATIVE_INT16);
    }

    auto output = chihaya::validate(path, "hello"); 
    EXPECT_EQ(output.type, chihaya::FLOAT);
    EXPECT_EQ(output.dimensions.size(), 2);
    EXPECT_EQ(output.dimensions[0], 10);
    EXPECT_EQ(output.dimensions[1], 7);
}

TEST_P(UnaryArithmeticTest, SeedErrors) {
    auto version = GetParam();

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        unary_arithmetic_opener(fhandle, "hello", "*", "right", { 10, 7 }, version, "STRING");
    }
    expect_error([&]() -> void { chihaya::validate(path, "hello"); }, "integer, float or boolean");

    {
        H5::H5File fhandle(path, H5F_ACC_RDWR);
        auto ghandle = fhandle.openGroup("hello");
        ghandle.unlink("seed");
    }
    expect_error([&]() -> void { chihaya::validate(path, "hello"); }, "expected a group at 'seed'");
}

TEST_P(UnaryArithmeticTest, SideErrors) {
    auto version = GetParam();

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = unary_arithmetic_opener(fhandle, "hello", "*", "right", { 10, 7 }, version, "FLOAT");
        ghandle.unlink("side");
    }
    expect_error([&]() -> void { chihaya::validate(path, "hello"); }, "expected a dataset at 'side'");

    {
        H5::H5File fhandle(path, H5F_ACC_RDWR);
        auto ghandle = fhandle.openGroup("hello");
        add_numeric_scalar<int>(ghandle, "side", 1, H5::PredType::NATIVE_INT16);
    }
    expect_error([&]() -> void { chihaya::validate(path, "hello"); }, "UTF-8 encoded string");

    {
        H5::H5File fhandle(path, H5F_ACC_RDWR);
        auto ghandle = fhandle.openGroup("hello");
        ghandle.unlink("side");
        add_string_scalar(ghandle, "side", "foo");
    }
    expect_error([&]() -> void { chihaya::validate(path, "hello"); }, "should be 'left' or 'right'");

    {
        H5::H5File fhandle(path, H5F_ACC_RDWR);
        auto ghandle = fhandle.openGroup("hello");
        ghandle.unlink("side");
        add_string_scalar(ghandle, "side", "none");
    }
    expect_error([&]() -> void { chihaya::validate(path, "hello"); }, "cannot be 'none'");
}

TEST_P(UnaryArithmeticTest, MethodErrors) {
    auto version = GetParam();

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = unary_arithmetic_opener(fhandle, "hello", "*", "right", { 10, 7 }, version, "FLOAT");
        ghandle.unlink("method");
    }
    expect_error([&]() -> void { chihaya::validate(path, "hello"); }, "expected a dataset at 'method'");

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = unary_arithmetic_opener(fhandle, "hello", "*", "right", { 10, 7 }, version, "FLOAT");
        ghandle.unlink("method");
        add_numeric_scalar<int>(ghandle, "method", 1, H5::PredType::NATIVE_INT32);
    }
    expect_error([&]() -> void { chihaya::validate(path, "hello"); }, "UTF-8 encoded string");

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = unary_arithmetic_opener(fhandle, "hello", "*", "right", { 10, 7 }, version, "FLOAT");
        ghandle.unlink("method");
        add_string_scalar(ghandle, "method", "foo");
    }
    expect_error([&]() -> void { chihaya::validate(path, "hello"); }, "unrecognized operation in 'method' (got 'foo')");
}

TEST_P(UnaryArithmeticTest, ValueErrors) {
    auto version = GetParam();

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        unary_arithmetic_opener(fhandle, "hello", "*", "right", { 10, 7 }, version, "FLOAT");
    }
    expect_error([&]() -> void { chihaya::validate(path, "hello"); }, "expected a dataset at 'value'");

    {
        H5::H5File fhandle(path, H5F_ACC_RDWR);
        auto ghandle = fhandle.openGroup("hello");
        auto dhandle = add_string_scalar(ghandle, "value", "WHEE");
        if (version >= 1100000) {
            add_string_attribute(dhandle, "type", "STRING");
        }
    }
    expect_error([&]() -> void { chihaya::validate(path, "hello"); }, "should be integer, float or boolean");

    {
        H5::H5File fhandle(path, H5F_ACC_RDWR);
        auto ghandle = fhandle.openGroup("hello");
        ghandle.unlink("value");

        hsize_t dims[2];
        dims[0] = 5;
        dims[1] = 5;
        H5::DataSpace dspace(2, dims);
        auto dhandle = ghandle.createDataSet("value", H5::PredType::NATIVE_INT, dspace);
        if (version >= 1100000) {
            add_string_attribute(dhandle, "type", "INTEGER");
        }
    }
    expect_error([&]() -> void { chihaya::validate(path, "hello"); }, "dataset should be scalar or 1-dimensional");
}

TEST_P(UnaryArithmeticTest, AlongErrors) {
    auto version = GetParam();

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = unary_arithmetic_opener(fhandle, "hello", "/", "left", { 19, 4 }, version, "INTEGER");
        auto dhandle = add_numeric_vector<int>(ghandle, "value", { 1, 2, 3, 4 }, H5::PredType::NATIVE_INT);
        if (version >= 1100000) {
            add_string_attribute(dhandle, "type", "INTEGER");
        }
    }
    expect_error([&]() -> void { chihaya::validate(path, "hello"); }, "expected a dataset at 'along'");

    {
        H5::H5File fhandle(path, H5F_ACC_RDWR);
        auto ghandle = fhandle.openGroup("hello");
        add_string_scalar(ghandle, "along", "WHEE");
    }
    if (version < 1100000) {
        expect_error([&]() -> void { chihaya::validate(path, "hello"); }, "'along' should be an integer dataset");
    } else {
        expect_error([&]() -> void { chihaya::validate(path, "hello"); }, "64-bit unsigned integer");
    }

    {
        H5::H5File fhandle(path, H5F_ACC_RDWR);
        auto ghandle = fhandle.openGroup("hello");
        ghandle.unlink("along");
        add_numeric_scalar<int>(ghandle, "along", -1, H5::PredType::NATIVE_INT);
    }
    if (version < 1100000) {
        expect_error([&]() -> void { chihaya::validate(path, "hello"); }, "'along' should be non-negative");
    } else {
        expect_error([&]() -> void { chihaya::validate(path, "hello"); }, "64-bit unsigned integer");
    }

    {
        H5::H5File fhandle(path, H5F_ACC_RDWR);
        auto ghandle = fhandle.openGroup("hello");
        ghandle.unlink("along");
        add_numeric_scalar<int>(ghandle, "along", 0, H5::PredType::NATIVE_UINT8);
    }
    expect_error([&]() -> void { chihaya::validate(path, "hello"); }, "length of 'value' dataset");
}

TEST_P(UnaryArithmeticTest, MissingErrors) {
    auto version = GetParam();

    if (version >= 1000000) {
        {
            H5::H5File fhandle(path, H5F_ACC_TRUNC);
            auto ghandle = unary_arithmetic_opener(fhandle, "hello", "^", "left", { 5, 19 }, version, "FLOAT");
            auto dhandle = add_numeric_vector<int>(ghandle, "value", { -1, -2, -3, -4 }, H5::PredType::NATIVE_INT32);
            if (version >= 1100000) {
                add_string_attribute(dhandle, "type", "FLOAT");
            }
            add_numeric_missing_placeholder(dhandle, 5, H5::PredType::NATIVE_FLOAT);
        }
        if (version < 1100000) {
            expect_error([&]() -> void { chihaya::validate(path, "hello"); }, "same type class");
        } else {
            expect_error([&]() -> void { chihaya::validate(path, "hello"); }, "same type as ");
        }
    }

    if (version >= 1100000) {
        {
            H5::H5File fhandle(path, H5F_ACC_TRUNC);
            auto ghandle = unary_arithmetic_opener(fhandle, "hello", "^", "left", { 5, 19 }, version, "FLOAT");
            auto dhandle = add_numeric_vector<int>(ghandle, "value", { -1, -2, -3, -4 }, H5::PredType::NATIVE_INT32);
            if (version >= 1100000) {
                add_string_attribute(dhandle, "type", "FLOAT");
            }
            add_numeric_missing_placeholder(dhandle, 5, H5::PredType::NATIVE_INT8);
        }
        expect_error([&]() -> void { chihaya::validate(path, "hello"); }, "same type as ");
    }
}

INSTANTIATE_TEST_SUITE_P(
    UnaryArithmetic,
    UnaryArithmeticTest,
    ::testing::Values(0, 1000000, 1100000)
);
