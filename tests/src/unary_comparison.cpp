#include <gtest/gtest.h>
#include "chihaya/chihaya.hpp"
#include "utils.h"

class UnaryComparisonTest : public ::testing::TestWithParam<int> {
public:
    UnaryComparisonTest() : path("Test_unary_comparison.h5") {}
protected:
    std::string path;

    static H5::Group unary_comparison_opener(H5::Group& handle, const std::string& name, const std::string& method, const std::string& side, const std::vector<int>& dimensions, int version, const std::string& type) {
        auto ghandle = operation_opener(handle, name, "unary comparison");
        add_version_string(ghandle, version);
        mock_array_opener(ghandle, "seed", dimensions, version, type);
        add_string_scalar(ghandle, "method", method);
        add_string_scalar(ghandle, "side", side);
        return ghandle;
    }
};

TEST_P(UnaryComparisonTest, ScalarUnary) {
    auto version = GetParam();

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = unary_comparison_opener(fhandle, "hello", "!=", "left", { 17, 21 }, version, "INTEGER");
        auto dhandle = add_numeric_scalar<double>(ghandle, "value", 2.5, H5::PredType::NATIVE_DOUBLE);
        if (version >= 1100000) {
            add_string_attribute(dhandle, "type", "FLOAT");
        }
    }

    auto output = test_validate(path, "hello"); 
    EXPECT_EQ(output.type, chihaya::BOOLEAN);
    EXPECT_EQ(output.dimensions.size(), 2);
    EXPECT_EQ(output.dimensions[0], 17);
    EXPECT_EQ(output.dimensions[1], 21);

    auto skipped = test_validate_skip(path, "hello");
    EXPECT_EQ(skipped.type, output.type);
    EXPECT_EQ(skipped.dimensions, output.dimensions);

    // Works with strings.
    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = unary_comparison_opener(fhandle, "hello", "!=", "left", { 17, 21 }, version, "STRING");
        auto dhandle = add_string_scalar(ghandle, "value", "FOO");
        if (version >= 1100000) {
            add_string_attribute(dhandle, "type", "STRING");
        }
    }

    output = test_validate(path, "hello"); 
    EXPECT_EQ(output.type, chihaya::BOOLEAN);
    EXPECT_EQ(output.dimensions.size(), 2);
    EXPECT_EQ(output.dimensions[0], 17);
    EXPECT_EQ(output.dimensions[1], 21);
}

TEST_P(UnaryComparisonTest, VectorUnary) {
    auto version = GetParam();

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = unary_comparison_opener(fhandle, "hello", ">", "left", { 5, 2 }, version, "INTEGER");
        add_numeric_scalar<int>(ghandle, "along", 0, H5::PredType::NATIVE_UINT8);
        auto dhandle = add_numeric_vector<int>(ghandle, "value", { 1, 2, 3, 4, 5 }, H5::PredType::NATIVE_INT8);
        if (version >= 1100000) {
            add_string_attribute(dhandle, "type", "INTEGER");
        }
    }

    auto output = test_validate(path, "hello"); 
    EXPECT_EQ(output.type, chihaya::BOOLEAN);
    EXPECT_EQ(output.dimensions.size(), 2);
    EXPECT_EQ(output.dimensions[0], 5);
    EXPECT_EQ(output.dimensions[1], 2);
}

TEST_P(UnaryComparisonTest, Missing) {
    auto version = GetParam();
    if (version < 1000000) {
        return;
    }

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = unary_comparison_opener(fhandle, "hello", "==", "right", { 5, 2 }, version, "INTEGER");
        add_numeric_scalar<int>(ghandle, "along", 0, H5::PredType::NATIVE_INT8);
        auto dhandle = add_numeric_scalar<int>(ghandle, "value", 11, H5::PredType::NATIVE_INT8);
        if (version >= 1100000) {
            add_string_attribute(dhandle, "type", "INTEGER");
            add_numeric_missing_placeholder(dhandle, 2, H5::PredType::NATIVE_INT8);
        } else {
            add_numeric_missing_placeholder(dhandle, 2, H5::PredType::NATIVE_INT32);
        }
    }

    auto output = test_validate(path, "hello"); 
    EXPECT_EQ(output.type, chihaya::BOOLEAN);
}

TEST_P(UnaryComparisonTest, SideErrors) {
    auto version = GetParam();

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = unary_comparison_opener(fhandle, "hello", "==", "right", { 15, 12 }, version, "INTEGER");
        ghandle.unlink("side");
    }
    expect_error(path, "hello", "expected a dataset at 'side'");

    {
        H5::H5File fhandle(path, H5F_ACC_RDWR);
        auto ghandle = fhandle.openGroup("hello");
        add_numeric_scalar<int>(ghandle, "side", 1, H5::PredType::NATIVE_INT);
    }
    expect_error(path, "hello", "can be represented by a UTF-8 encoded string");

    {
        H5::H5File fhandle(path, H5F_ACC_RDWR);
        auto ghandle = fhandle.openGroup("hello");
        ghandle.unlink("side");
        add_string_scalar(ghandle, "side", "foo");
    }
    expect_error(path, "hello", "either 'left' or 'right'");
}

TEST_P(UnaryComparisonTest, MethodErrors) {
    auto version = GetParam();

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = unary_comparison_opener(fhandle, "hello", "==", "right", { 15, 12 }, version, "INTEGER");
        ghandle.unlink("method");
    }
    expect_error(path, "hello", "expected a dataset at 'method'");

    {
        H5::H5File fhandle(path, H5F_ACC_RDWR);
        auto ghandle = fhandle.openGroup("hello");
        add_numeric_scalar<int>(ghandle, "method", 1, H5::PredType::NATIVE_INT);
    }
    expect_error(path, "hello", "can be represented by a UTF-8 encoded string");

    {
        H5::H5File fhandle(path, H5F_ACC_RDWR);
        auto ghandle = fhandle.openGroup("hello");
        ghandle.unlink("method");
        add_string_scalar(ghandle, "method", "foo");
    }
    expect_error(path, "hello", "unrecognized operation");
}

TEST_P(UnaryComparisonTest, ValueErrors) {
    auto version = GetParam();

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        unary_comparison_opener(fhandle, "hello", ">=", "right", { 15, 12 }, version, "INTEGER");
    }
    expect_error(path, "hello", "expected a dataset at 'value'");

    {
        H5::H5File fhandle(path, H5F_ACC_RDWR);
        auto ghandle = fhandle.openGroup("hello");
        auto dhandle = add_string_scalar(ghandle, "value", "WHEE");
        if (version >= 1100000) {
            add_string_attribute(dhandle, "type", "STRING");
        }
    }
    expect_error(path, "hello", "both or neither");

    {
        H5::H5File fhandle(path, H5F_ACC_RDWR);
        auto ghandle = fhandle.openGroup("hello");
        ghandle.unlink("value");

        hsize_t dims[2];
        dims[0] = 5;
        dims[1] = 5;
        H5::DataSpace dspace(2, dims);
        auto dhandle = ghandle.createDataSet("value", H5::PredType::NATIVE_INT32, dspace);
        if (version >= 1100000) {
            add_string_attribute(dhandle, "type", "INTEGER");
        }
    }
    expect_error(path, "hello", "dataset should be scalar or 1-dimensional");
}

TEST_P(UnaryComparisonTest, AlongErrors) {
    auto version = GetParam();

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = unary_comparison_opener(fhandle, "hello", "<=", "right", { 21, 4 }, version, "FLOAT");
        auto dhandle = add_numeric_vector<int>(ghandle, "value", { 1, 2, 3, 4 }, H5::PredType::NATIVE_UINT8);
        if (version >= 1100000) {
            add_string_attribute(dhandle, "type", "INTEGER");
        }
    }
    expect_error(path, "hello", "expected a dataset at 'along'");

    {
        H5::H5File fhandle(path, H5F_ACC_RDWR);
        auto ghandle = fhandle.openGroup("hello");
        add_string_scalar(ghandle, "along", "WHEE");
    }
    if (version < 1100000) {
        expect_error(path, "hello", "'along' should be an integer dataset");
    } else {
        expect_error(path, "hello", "64-bit unsigned integer");
    }

    {
        H5::H5File fhandle(path, H5F_ACC_RDWR);
        auto ghandle = fhandle.openGroup("hello");
        ghandle.unlink("along");
        add_numeric_scalar<int>(ghandle, "along", -1, H5::PredType::NATIVE_INT);
    }
    if (version < 1100000) {
        expect_error(path, "hello", "'along' should be non-negative");
    } else {
        expect_error(path, "hello", "64-bit unsigned integer");
    }

    {
        H5::H5File fhandle(path, H5F_ACC_RDWR);
        auto ghandle = fhandle.openGroup("hello");
        ghandle.unlink("along");
        add_numeric_scalar<int>(ghandle, "along", 0, H5::PredType::NATIVE_UINT8);
    }
    expect_error(path, "hello", "length of 'value' dataset");
}

TEST_P(UnaryComparisonTest, MissingErrors) {
    auto version = GetParam();

    if (version >= 1000000) {
        {
            H5::H5File fhandle(path, H5F_ACC_TRUNC);
            auto ghandle = unary_comparison_opener(fhandle, "hello", ">", "left", { 5, 19 }, version, "FLOAT");
            auto dhandle = add_numeric_vector<int>(ghandle, "value", { -1, -2, -3, -4 }, H5::PredType::NATIVE_INT32);
            if (version >= 1100000) {
                add_string_attribute(dhandle, "type", "FLOAT");
            }
            add_numeric_missing_placeholder(dhandle, 5, H5::PredType::NATIVE_FLOAT);
        }
        if (version < 1100000) {
            expect_error(path, "hello", "same type class");
        } else {
            expect_error(path, "hello", "same type as ");
        }
    }

    if (version >= 1100000) {
        {
            H5::H5File fhandle(path, H5F_ACC_TRUNC);
            auto ghandle = unary_comparison_opener(fhandle, "hello", "<", "left", { 5, 19 }, version, "FLOAT");
            auto dhandle = add_numeric_vector<int>(ghandle, "value", { -1, -2, -3, -4 }, H5::PredType::NATIVE_INT32);
            if (version >= 1100000) {
                add_string_attribute(dhandle, "type", "FLOAT");
            }
            add_numeric_missing_placeholder(dhandle, 5, H5::PredType::NATIVE_INT8);
        }
        expect_error(path, "hello", "same type as ");
    }
}

INSTANTIATE_TEST_SUITE_P(
    UnaryComparison,
    UnaryComparisonTest,
    ::testing::Values(0, 1000000, 1100000)
);
