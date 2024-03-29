#include <gtest/gtest.h>
#include "chihaya/chihaya.hpp"
#include "utils.h"

class ConstantArrayTest : public ::testing::TestWithParam<int> {
public:
    ConstantArrayTest() : path("Test_constant_array.h5") {}
protected:
    std::string path;

    H5::Group constant_array_opener(H5::Group& handle, std::string name, const std::vector<int>& dimensions, int version) {
        auto ghandle = array_opener(handle, name, "constant array");
        add_version_string(ghandle, version);
        if (version < 1100000) {
            add_numeric_vector(ghandle, "dimensions", dimensions, H5::PredType::NATIVE_INT);
        } else {
            add_numeric_vector(ghandle, "dimensions", dimensions, H5::PredType::NATIVE_UINT32);
        }
        return ghandle;
    }
};

TEST_P(ConstantArrayTest, Basic) {
    auto version = GetParam();

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = constant_array_opener(fhandle, "constant", { 20, 17 }, version);
        auto dhandle = add_numeric_scalar(ghandle, "value", 1, H5::PredType::NATIVE_INT32);
        if (version >= 1100000) {
            add_string_attribute(dhandle, "type", "INTEGER");
        }
    }
    {
        auto output = test_validate(path, "constant"); 
        EXPECT_EQ(output.type, chihaya::INTEGER);
        const auto& dims = output.dimensions;
        EXPECT_EQ(dims.size(), 2);
        EXPECT_EQ(dims[0], 20);
        EXPECT_EQ(dims[1], 17);

        auto skipped = test_validate_skip(path, "constant");
        EXPECT_EQ(skipped.type, output.type);
        EXPECT_EQ(skipped.dimensions, output.dimensions);
    }

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = constant_array_opener(fhandle, "constant", { 5, 17 }, version);
        auto dhandle = add_numeric_scalar(ghandle, "value", 2.5, H5::PredType::NATIVE_DOUBLE);
        if (version >= 1100000) {
            add_string_attribute(dhandle, "type", "FLOAT");
        }
    }
    {
        auto output = test_validate(path, "constant"); 
        EXPECT_EQ(output.type, chihaya::FLOAT);
        const auto& dims = output.dimensions;
        EXPECT_EQ(dims.size(), 2);
        EXPECT_EQ(dims[0], 5);
        EXPECT_EQ(dims[1], 17);

        auto skipped = test_validate_skip(path, "constant");
        EXPECT_EQ(skipped.type, output.type);
        EXPECT_EQ(skipped.dimensions, output.dimensions);
    }

    // Throwing in some strings.
    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = constant_array_opener(fhandle, "constant", { 50, 10 }, 1);
        auto dhandle = add_string_scalar(ghandle, "value", "FOOBAR");
        if (version >= 1100000) {
            add_string_attribute(dhandle, "type", "STRING");
        }
    }
    {
        auto output = test_validate(path, "constant"); 
        EXPECT_EQ(output.type, chihaya::STRING);
    }
}

TEST_P(ConstantArrayTest, Missing) {
    auto version = GetParam();

    if (version == 1000000) {
        {
            H5::H5File fhandle(path, H5F_ACC_TRUNC);
            auto ghandle = constant_array_opener(fhandle, "constant", { 20, 17 }, version);
            auto dhandle = add_numeric_scalar(ghandle, "value", 1, H5::PredType::NATIVE_UINT32);
            add_numeric_missing_placeholder(dhandle, 1, H5::PredType::NATIVE_UINT8);
        }
        auto output = test_validate(path, "constant"); 
        EXPECT_EQ(output.type, chihaya::INTEGER);
    } else if (version >= 1100000) {
        {
            H5::H5File fhandle(path, H5F_ACC_TRUNC);
            auto ghandle = constant_array_opener(fhandle, "constant", { 20, 17 }, version);
            auto dhandle = add_numeric_scalar(ghandle, "value", 1, H5::PredType::NATIVE_UINT8);
            add_numeric_missing_placeholder(dhandle, 1, H5::PredType::NATIVE_UINT8);
            add_string_attribute(dhandle, "type", "INTEGER");
        }
        auto output = test_validate(path, "constant"); 
        EXPECT_EQ(output.type, chihaya::INTEGER);
    }

    // String placeholders only require the same datatype class.
    if (version >= 1000000) {
        {
            H5::H5File fhandle(path, H5F_ACC_TRUNC);
            auto ghandle = constant_array_opener(fhandle, "constant", { 20, 17 }, version);
            auto dhandle = add_string_scalar(ghandle, "value", "foo");
            add_string_missing_placeholder(dhandle, "bar", /* len = */ 10);
            add_string_attribute(dhandle, "type", "STRING");
        }
        auto output = test_validate(path, "constant"); 
        EXPECT_EQ(output.type, chihaya::STRING);
    }
}

TEST_P(ConstantArrayTest, Errors) {
    auto version = GetParam();

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = constant_array_opener(fhandle, "constant", { 20, 17 }, version);
        ghandle.unlink("dimensions");
        add_numeric_vector<int>(ghandle, "dimensions", {20, 17}, H5::PredType::NATIVE_DOUBLE);
    }
    if (version < 1100000) {
        expect_error(path, "constant", "'dimensions' should be integer");
    } else {
        expect_error(path, "constant", "64-bit unsigned integer");
    }

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        constant_array_opener(fhandle, "constant", {}, version);
    }
    expect_error(path, "constant", "should have non-zero");

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = array_opener(fhandle, "constant", "constant array");
        add_version_string(ghandle, version);
        add_numeric_vector<int>(ghandle, "dimensions", { -1 }, H5::PredType::NATIVE_INT);
    }
    if (version < 1100000) {
        expect_error(path, "constant", "non-negative");
    } else {
        expect_error(path, "constant", "64-bit unsigned integer");
    }

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = constant_array_opener(fhandle, "constant", { 50, 10 }, 1);
        ghandle.createGroup("value");
    }
    expect_error(path, "constant", "expected a dataset at 'value'");

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = constant_array_opener(fhandle, "constant", { 50, 10 }, 1);
        add_numeric_vector<int>(ghandle, "value", { 20, 20 }, H5::PredType::NATIVE_INT);
    }
    expect_error(path, "constant", "should be a scalar");

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = constant_array_opener(fhandle, "constant", { 50, 10 }, 1);
        H5::StrType stype(0, H5T_VARIABLE);
        auto dhandle = ghandle.createDataSet("value", stype, H5S_SCALAR); 
        if (version >= 1100000) {
            add_string_attribute(dhandle, "type", "STRING");
        }
    }
    expect_error(path, "constant", "NULL pointer");
}

TEST_P(ConstantArrayTest, MissingErrors) {
    auto version = GetParam();

    if (version >= 1000000) {
        {
            H5::H5File fhandle(path, H5F_ACC_TRUNC);
            auto ghandle = constant_array_opener(fhandle, "constant", { 20, 17 }, version);
            auto dhandle = add_numeric_scalar(ghandle, "value", 1, H5::PredType::NATIVE_INT);
            if (version >= 1100000) {
                add_string_attribute(dhandle, "type", "INTEGER");
            }
            add_numeric_missing_placeholder(dhandle, 1, H5::PredType::NATIVE_DOUBLE);
        }
        if (version >= 1100000) {
            expect_error(path, "constant", "have the same type as");
        } else {
            expect_error(path, "constant", "have the same type class");
        }
    }

    if (version >= 1100000) {
        {
            H5::H5File fhandle(path, H5F_ACC_TRUNC);
            auto ghandle = constant_array_opener(fhandle, "constant", { 20, 17 }, version);
            auto dhandle = add_numeric_scalar(ghandle, "value", 1, H5::PredType::NATIVE_INT32);
            add_numeric_missing_placeholder(dhandle, 1, H5::PredType::NATIVE_INT8);
            add_string_attribute(dhandle, "type", "INTEGER");
        }
        expect_error(path, "constant", "same type as");
    }
}

INSTANTIATE_TEST_SUITE_P(
    ConstantArray,
    ConstantArrayTest,
    ::testing::Values(0, 1000000, 1100000)
);
