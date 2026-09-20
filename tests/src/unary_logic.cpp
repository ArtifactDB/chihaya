#include <gtest/gtest.h>

#include <string>
#include <vector>
#include <cstddef>

#include "H5Cpp.h"
#include "ritsuko/ritsuko.hpp"
#include "chihaya/unary_logic.hpp"

#include "utils.h"

static H5::Group unary_logic_opener(
    H5::Group& handle,
    const std::string& name,
    const std::string& method,
    const std::vector<std::size_t>& dimensions,
    const ritsuko::Version& version,
    const std::string& type
) {
    auto ghandle = operation_opener(handle, name, "unary logic");
    add_version_string(ghandle, version);
    mock_array_opener(ghandle, "seed", dimensions, version, type);
    add_string_scalar(ghandle, "method", method);
    return ghandle;
}

/*******************************/

class UnaryLogicPassTest : public ::testing::TestWithParam<std::tuple<ritsuko::Version, bool> > {};

TEST_P(UnaryLogicPassTest, PureUnary) {
    auto path = define_test_path("unary_logic");
    auto param = GetParam();
    auto version = std::get<0>(param);
    auto deets = std::get<1>(param);

    std::vector<std::size_t> dims{ 12, 32 };
    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        unary_logic_opener(fhandle, "hello", "!", dims, version, "INTEGER");
    }

    auto output = test_validate(path, "hello", deets); 
    EXPECT_EQ(output.type, chihaya::BOOLEAN);
    EXPECT_EQ(output.dimensions, dims);
}

TEST_P(UnaryLogicPassTest, Scalar) {
    auto path = define_test_path("unary_logic");
    auto param = GetParam();
    auto version = std::get<0>(param);
    auto deets = std::get<1>(param);

    std::vector<std::size_t> dims{ 31, 13 };
    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = unary_logic_opener(fhandle, "hello", "||", dims, version, "INTEGER");
        add_string_scalar(ghandle, "side", "left");
        auto dhandle = add_numeric_scalar<double>(ghandle, "value", 2.5, H5::PredType::NATIVE_DOUBLE);
        if (version.ge(1, 1, 0)) {
            add_string_attribute(dhandle, "type", "FLOAT");
        }
    }

    auto output = test_validate(path, "hello", deets); 
    EXPECT_EQ(output.type, chihaya::BOOLEAN);
    EXPECT_EQ(output.dimensions, dims);
}

TEST_P(UnaryLogicPassTest, Vector) {
    auto path = define_test_path("unary_logic");
    auto param = GetParam();
    auto version = std::get<0>(param);
    auto deets = std::get<1>(param);

    std::vector<std::size_t> dims{ 5, 17 };
    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = unary_logic_opener(fhandle, "hello", "&&", dims, version, "FLOAT");
        add_string_scalar(ghandle, "side", "right");
        add_numeric_scalar<int>(ghandle, "along", 0, H5::PredType::NATIVE_UINT32);
        auto dhandle = add_numeric_vector<double>(ghandle, "value", { 1, 2, 3, 4, 5 }, H5::PredType::NATIVE_DOUBLE);
        if (version.ge(1, 1, 0)) {
            add_string_attribute(dhandle, "type", "FLOAT");
        }
    }

    auto output = test_validate(path, "hello", deets); 
    EXPECT_EQ(output.type, chihaya::BOOLEAN);
    EXPECT_EQ(output.dimensions, dims);
}

TEST_P(UnaryLogicPassTest, Missing) {
    auto path = define_test_path("unary_logic");
    auto param = GetParam();
    auto version = std::get<0>(param);
    auto deets = std::get<1>(param);

    if (version.lt(1, 0, 0)) {
        return;
    }

    std::vector<std::size_t> dims{ 29, 11 };
    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = unary_logic_opener(fhandle, "hello", "||", dims, version, "INTEGER");
        add_string_scalar(ghandle, "side", "left");
        auto dhandle = add_numeric_scalar<double>(ghandle, "value", 2.5, H5::PredType::NATIVE_DOUBLE);
        if (version.ge(1, 1, 0)) {
            add_string_attribute(dhandle, "type", "FLOAT");
        }
        add_numeric_missing_placeholder(dhandle, 2.0, H5::PredType::NATIVE_DOUBLE);
    }

    auto output = test_validate(path, "hello", deets); 
    EXPECT_EQ(output.type, chihaya::BOOLEAN);
    EXPECT_EQ(output.dimensions, dims);
}

INSTANTIATE_TEST_SUITE_P(
    UnaryLogic,
    UnaryLogicPassTest,
    ::testing::Combine(
        spawn_all_versions(),
        ::testing::Values(false, true)
    )
);

/*******************************/

class UnaryLogicErrorTest : public ::testing::TestWithParam<ritsuko::Version> {};

TEST_P(UnaryLogicErrorTest, Seed) {
    auto path = define_test_path("unary_logic");
    auto version = GetParam();

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        unary_logic_opener(fhandle, "hello", "!", { 10, 7 }, version, "STRING");
    }
    expect_error(path, "hello", "integer, float or boolean");
}

TEST_P(UnaryLogicErrorTest, Side) {
    auto path = define_test_path("unary_logic");
    auto version = GetParam();

    // Test that we check that 'side' is a scalar string dataset.
    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = unary_logic_opener(fhandle, "hello", "&&", { 10, 7 }, version, "BOOLEAN");
        add_numeric_scalar<int>(ghandle, "side", 1, H5::PredType::NATIVE_INT);
    }
    expect_error(path, "hello", "UTF-8 encoded string");

    {
        H5::H5File fhandle(path, H5F_ACC_RDWR);
        auto ghandle = fhandle.openGroup("hello");
        ghandle.unlink("side");
        add_string_scalar(ghandle, "side", "foo");
    }
    expect_error(path, "hello", "'left' or 'right'");
}

TEST_P(UnaryLogicErrorTest, Method) {
    auto path = define_test_path("unary_logic");
    auto version = GetParam();

    // Test that we check that 'method' is a scalar string dataset.
    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = unary_logic_opener(fhandle, "hello", "||", { 13, 19 }, version, "INTEGER");
        ghandle.unlink("method");
        add_numeric_scalar<int>(ghandle, "method", 1, H5::PredType::NATIVE_INT);
    }
    expect_error(path, "hello", "UTF-8 encoded string");

    {
        H5::H5File fhandle(path, H5F_ACC_RDWR);
        auto ghandle = fhandle.openGroup("hello");
        ghandle.unlink("method");
        add_string_scalar(ghandle, "method", "foo");
    }
    expect_error(path, "hello", "unrecognized operation in 'method'");
}

TEST_P(UnaryLogicErrorTest, Value) {
    auto path = define_test_path("unary_logic");
    auto version = GetParam();

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = unary_logic_opener(fhandle, "hello", "&&", { 10, 7 }, version, "FLOAT");
        add_string_scalar(ghandle, "side", "left");
        auto dhandle = add_string_scalar(ghandle, "value", "WHEE");
        if (version.ge(1, 1, 0)) {
            add_string_attribute(dhandle, "type", "STRING");
        }
    }
    expect_error(path, "hello", "integer, float or boolean");

    if (version.ge(1, 1, 0)) {
        // Test that we actually check for a scalar 'type'.
        {
            H5::H5File fhandle(path, H5F_ACC_TRUNC);
            auto ghandle = unary_logic_opener(fhandle, "hello", "&&", { 10, 7 }, version, "INTEGER");
            add_string_scalar(ghandle, "side", "left");
            auto dhandle = add_numeric_vector<int>(ghandle, "value", { 1, 2, 3, 4, 5 }, H5::PredType::NATIVE_UINT8);
            constexpr hsize_t one = 1;
            dhandle.createAttribute("type", H5::StrType(0, H5T_VARIABLE), H5::DataSpace(1, &one));
        }
        expect_error(path, "hello", "scalar");

        // Test that we actually check the 'type' is consistent with the dataset's type.
        {
            H5::H5File fhandle(path, H5F_ACC_TRUNC);
            auto ghandle = unary_logic_opener(fhandle, "hello", "||", { 10, 7 }, version, "INTEGER");
            add_string_scalar(ghandle, "side", "right");
            auto dhandle = add_numeric_vector<int>(ghandle, "value", { 1, 2, 3, 4, 5 }, H5::PredType::NATIVE_INT64);
            add_string_attribute(dhandle, "type", "BOOLEAN");
        }
        expect_error(path, "hello", "8-bit signed integer");
    }

    {
        H5::H5File fhandle(path, H5F_ACC_RDWR);
        auto ghandle = fhandle.openGroup("hello");
        ghandle.unlink("value");
        hsize_t dims[2] = { 5, 5 };
        auto dhandle = ghandle.createDataSet("value", H5::PredType::NATIVE_INT16, H5::DataSpace(2, dims));
        if (version.ge(1, 1, 0)) {
            add_string_attribute(dhandle, "type", "INTEGER");
        }
    }
    expect_error(path, "hello", "dataset should be scalar or 1-dimensional");
}

TEST_P(UnaryLogicErrorTest, Along) {
    auto path = define_test_path("unary_logic");
    auto version = GetParam();

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = unary_logic_opener(fhandle, "hello", "&&", { 19, 4 }, version, "FLOAT");
        add_string_scalar(ghandle, "side", "left");
        auto dhandle = add_numeric_vector<int>(ghandle, "value", { 1, 2, 3, 4 }, H5::PredType::NATIVE_INT32);
        if (version.ge(1, 1, 0)) {
            add_string_attribute(dhandle, "type", "INTEGER");
        }
        add_string_scalar(ghandle, "along", "WHEE");
    }
    if (version.lt(1, 1, 0)) {
        expect_error(path, "hello", "expected an integer type");
    } else {
        expect_error(path, "hello", "64-bit unsigned integer");
    }

    {
        H5::H5File fhandle(path, H5F_ACC_RDWR);
        auto ghandle = fhandle.openGroup("hello");
        ghandle.unlink("along");
        add_numeric_scalar<int>(ghandle, "along", 0, H5::PredType::NATIVE_UINT8);
    }
    expect_error(path, "hello", "dimension specified in 'along'");
}

TEST_P(UnaryLogicErrorTest, Missing) {
    auto path = define_test_path("unary_logic");
    auto version = GetParam();

    if (version.le(1, 0, 0)) {
        return;
    }

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = unary_logic_opener(fhandle, "hello", "&&", { 5, 19 }, version, "INTEGER");
        add_string_scalar(ghandle, "side", "left");
        auto dhandle = add_numeric_vector<int>(ghandle, "value", { -1, -2, -3, -4 }, H5::PredType::NATIVE_INT32);
        if (version.ge(1, 1, 0)) {
            add_string_attribute(dhandle, "type", "FLOAT");
        }
        add_numeric_missing_placeholder(dhandle, 5, H5::PredType::NATIVE_FLOAT);
    }
    if (version.lt(1, 1, 0)) {
        expect_error(path, "hello", "same datatype class");
    } else {
        expect_error(path, "hello", "same datatype as ");
    }
}

INSTANTIATE_TEST_SUITE_P(
    UnaryLogic,
    UnaryLogicErrorTest,
    spawn_all_versions()
);
