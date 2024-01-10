#include <gtest/gtest.h>
#include "chihaya/chihaya.hpp"
#include "utils.h"

class UnaryLogicTest : public ::testing::TestWithParam<int> {
public:
    UnaryLogicTest() : path("Test_unary_logic.h5") {}
protected:
    std::string path;

    static H5::Group unary_logic_opener(H5::Group& handle, const std::string& name, const std::string& method, const std::vector<int>& dimensions, int version, const std::string& type) {
        auto ghandle = operation_opener(handle, name, "unary logic");
        add_version_string(ghandle, version);
        mock_array_opener(ghandle, "seed", dimensions, version, type);
        add_string_scalar(ghandle, "method", method);
        return ghandle;
    }
};

TEST_P(UnaryLogicTest, PureUnary) {
    auto version = GetParam();

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        unary_logic_opener(fhandle, "hello", "!", { 12, 32 }, version, "INTEGER");
    }

    auto output = chihaya::validate(path, "hello"); 
    EXPECT_EQ(output.type, chihaya::BOOLEAN);
    EXPECT_EQ(output.dimensions.size(), 2);
    EXPECT_EQ(output.dimensions[0], 12);
    EXPECT_EQ(output.dimensions[1], 32);
}

TEST_P(UnaryLogicTest, ScalarUnary) {
    auto version = GetParam();

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = unary_logic_opener(fhandle, "hello", "||", { 13, 19 }, version, "INTEGER");
        add_string_scalar(ghandle, "side", "left");
        auto dhandle = add_numeric_scalar<double>(ghandle, "value", 2.5, H5::PredType::NATIVE_DOUBLE);
        if (version >= 1100000) {
            add_string_attribute(dhandle, "type", "FLOAT");
        }
    }

    auto output = chihaya::validate(path, "hello"); 
    EXPECT_EQ(output.type, chihaya::BOOLEAN);
    EXPECT_EQ(output.dimensions.size(), 2);
    EXPECT_EQ(output.dimensions[0], 13);
    EXPECT_EQ(output.dimensions[1], 19);
}

TEST_P(UnaryLogicTest, VectorUnary) {
    auto version = GetParam();

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = unary_logic_opener(fhandle, "hello", "&&", { 5, 17 }, version, "FLOAT");
        add_string_scalar(ghandle, "side", "right");
        add_numeric_scalar<int>(ghandle, "along", 0, H5::PredType::NATIVE_UINT32);
        auto dhandle = add_numeric_vector<double>(ghandle, "value", { 1, 2, 3, 4, 5 }, H5::PredType::NATIVE_DOUBLE);
        if (version >= 1100000) {
            add_string_attribute(dhandle, "type", "FLOAT");
        }
    }

    auto output = chihaya::validate(path, "hello"); 
    EXPECT_EQ(output.type, chihaya::BOOLEAN);
    EXPECT_EQ(output.dimensions.size(), 2);
    EXPECT_EQ(output.dimensions[0], 5);
    EXPECT_EQ(output.dimensions[1], 17);
}

TEST_P(UnaryLogicTest, CheckMissing) {
    auto version = GetParam();
    if (version < 1000000) {
        return;
    }

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = unary_logic_opener(fhandle, "hello", "||", { 13, 19 }, version, "INTEGER");
        add_string_scalar(ghandle, "side", "left");
        auto dhandle = add_numeric_scalar<double>(ghandle, "value", 2.5, H5::PredType::NATIVE_DOUBLE);
        if (version >= 1100000) {
            add_string_attribute(dhandle, "type", "FLOAT");
        }
        add_numeric_missing_placeholder(dhandle, 2.0, H5::PredType::NATIVE_DOUBLE);
    }

    auto output = chihaya::validate(path, "hello"); 
    EXPECT_EQ(output.type, chihaya::BOOLEAN);
    EXPECT_EQ(output.dimensions.size(), 2);
    EXPECT_EQ(output.dimensions[0], 13);
    EXPECT_EQ(output.dimensions[1], 19);
}

TEST_P(UnaryLogicTest, SeedErrors) {
    auto version = GetParam();

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        unary_logic_opener(fhandle, "hello", "!", { 10, 7 }, version, "STRING");
    }
    expect_error([&]() -> void { chihaya::validate(path, "hello"); }, "integer, float or boolean");

    {
        H5::H5File fhandle(path, H5F_ACC_RDWR);
        auto ghandle = fhandle.openGroup("hello");
        ghandle.unlink("seed");
    }
    expect_error([&]() -> void { chihaya::validate(path, "hello"); }, "expected a group at 'seed'");
}

TEST_P(UnaryLogicTest, SideErrors) {
    auto version = GetParam();

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        unary_logic_opener(fhandle, "hello", "&&", { 10, 7 }, version, "BOOLEAN");
    }
    expect_error([&]() -> void { chihaya::validate(path, "hello"); }, "expected a dataset at 'side'");

    {
        H5::H5File fhandle(path, H5F_ACC_RDWR);
        auto ghandle = fhandle.openGroup("hello");
        add_numeric_scalar<int>(ghandle, "side", 1, H5::PredType::NATIVE_INT);
    }
    expect_error([&]() -> void { chihaya::validate(path, "hello"); }, "UTF-8 encoded string");

    {
        H5::H5File fhandle(path, H5F_ACC_RDWR);
        auto ghandle = fhandle.openGroup("hello");
        ghandle.unlink("side");
        add_string_scalar(ghandle, "side", "foo");
    }
    expect_error([&]() -> void { chihaya::validate(path, "hello"); }, "'left' or 'right'");
}

TEST_P(UnaryLogicTest, MethodErrors) {
    auto version = GetParam();

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = unary_logic_opener(fhandle, "hello", "||", { 13, 19 }, version, "INTEGER");
        ghandle.unlink("method");
    }
    expect_error([&]() -> void { chihaya::validate(path, "hello"); }, "expected a dataset at 'method'");

    {
        H5::H5File fhandle(path, H5F_ACC_RDWR);
        auto ghandle = fhandle.openGroup("hello");
        add_numeric_scalar<int>(ghandle, "method", 1, H5::PredType::NATIVE_INT);
    }
    expect_error([&]() -> void { chihaya::validate(path, "hello"); }, "UTF-8 encoded string");

    {
        H5::H5File fhandle(path, H5F_ACC_RDWR);
        auto ghandle = fhandle.openGroup("hello");
        ghandle.unlink("method");
        add_string_scalar(ghandle, "method", "foo");
    }
    expect_error([&]() -> void { chihaya::validate(path, "hello"); }, "unrecognized operation in 'method'");
}

TEST_P(UnaryLogicTest, ValueErrors) {
    auto version = GetParam();

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = unary_logic_opener(fhandle, "hello", "&&", { 10, 7 }, version, "FLOAT");
        add_string_scalar(ghandle, "side", "left");
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
    expect_error([&]() -> void { chihaya::validate(path, "hello"); }, "integer, float or boolean");

    {
        H5::H5File fhandle(path, H5F_ACC_RDWR);
        auto ghandle = fhandle.openGroup("hello");
        ghandle.unlink("value");

        hsize_t dims[2] = { 5, 5 };
        H5::DataSpace dspace(2, dims);
        auto dhandle = ghandle.createDataSet("value", H5::PredType::NATIVE_INT16, dspace);
        if (version >= 1100000) {
            add_string_attribute(dhandle, "type", "INTEGER");
        }
    }
    expect_error([&]() -> void { chihaya::validate(path, "hello"); }, "dataset should be scalar or 1-dimensional");
}

TEST_P(UnaryLogicTest, AlongErrors) {
    auto version = GetParam();

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = unary_logic_opener(fhandle, "hello", "&&", { 19, 4 }, version, "FLOAT");
        add_string_scalar(ghandle, "side", "left");
        auto dhandle = add_numeric_vector<int>(ghandle, "value", { 1, 2, 3, 4 }, H5::PredType::NATIVE_INT32);
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

TEST_P(UnaryLogicTest, MissingErrors) {
    auto version = GetParam();

    if (version >= 1000000) {
        {
            H5::H5File fhandle(path, H5F_ACC_TRUNC);
            auto ghandle = unary_logic_opener(fhandle, "hello", "&&", { 5, 19 }, version, "INTEGER");
            add_string_scalar(ghandle, "side", "left");
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
            auto ghandle = unary_logic_opener(fhandle, "hello", "||", { 5, 19 }, version, "FLOAT");
            add_string_scalar(ghandle, "side", "left");
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
    UnaryLogic,
    UnaryLogicTest,
    ::testing::Values(0, 1000000, 1100000)
);
