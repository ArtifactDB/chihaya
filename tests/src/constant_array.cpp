#include <gtest/gtest.h>

#include <cstddef>
#include <string>
#include <vector>

#include "H5Cpp.h"
#include "ritsuko/ritsuko.hpp"
#include "chihaya/constant_array.hpp"

#include "utils.h"

static H5::Group constant_array_opener(H5::Group& handle, std::string name, const std::vector<std::size_t>& dimensions, const ritsuko::Version& version) {
    auto ghandle = array_opener(handle, name, "constant array");
    add_version_string(ghandle, version);
    if (version.lt(1, 1, 0)) {
        add_numeric_vector(ghandle, "dimensions", dimensions, H5::PredType::NATIVE_INT);
    } else {
        add_numeric_vector(ghandle, "dimensions", dimensions, H5::PredType::NATIVE_UINT32);
    }
    return ghandle;
}

/***********************************/

class ConstantArrayPassTest : public ::testing::TestWithParam<std::tuple<ritsuko::Version, bool> > {};

TEST_P(ConstantArrayPassTest, Integer) {
    auto path = define_test_path("constant_array");
    auto params = GetParam();
    auto version = std::get<0>(params);
    auto deets = std::get<1>(params);

    std::vector<std::size_t> dimensions{ 20, 17 };
    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = constant_array_opener(fhandle, "constant", { 20, 17 }, version);
        auto dhandle = add_numeric_scalar(ghandle, "value", 1, H5::PredType::NATIVE_INT32);
        if (version.ge(1, 1, 0)) {
            add_string_attribute(dhandle, "type", "INTEGER");
        }
    }

    auto output = test_validate(path, "constant", deets); 
    EXPECT_EQ(output.type, chihaya::INTEGER);
    EXPECT_EQ(output.dimensions, dimensions);
}

TEST_P(ConstantArrayPassTest, Float) {
    auto path = define_test_path("constant_array");
    auto params = GetParam();
    auto version = std::get<0>(params);
    auto deets = std::get<1>(params);

    std::vector<std::size_t> dimensions{ 5, 13 };
    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = constant_array_opener(fhandle, "constant", dimensions, version);
        auto dhandle = add_numeric_scalar(ghandle, "value", 2.5, H5::PredType::NATIVE_DOUBLE);
        if (version.ge(1, 1, 0)) {
            add_string_attribute(dhandle, "type", "FLOAT");
        }
    }

    auto output = test_validate(path, "constant", deets); 
    EXPECT_EQ(output.type, chihaya::FLOAT);
    EXPECT_EQ(output.dimensions, dimensions);
}

TEST_P(ConstantArrayPassTest, String) {
    auto path = define_test_path("constant_array");
    auto params = GetParam();
    auto version = std::get<0>(params);
    auto deets = std::get<1>(params);

    std::vector<std::size_t> dimensions{ 50, 10 };
    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = constant_array_opener(fhandle, "constant", dimensions, version);
        auto dhandle = add_string_scalar(ghandle, "value", "FOOBAR");
        if (version.ge(1, 1, 0)) {
            add_string_attribute(dhandle, "type", "STRING");
        }
    }

    auto output = test_validate(path, "constant", deets); 
    EXPECT_EQ(output.type, chihaya::STRING);
    EXPECT_EQ(output.dimensions, dimensions);
}

TEST_P(ConstantArrayPassTest, MissingInteger) {
    auto path = define_test_path("constant_array");
    auto params = GetParam();
    auto version = std::get<0>(params);
    auto deets = std::get<1>(params);

    std::vector<std::size_t> dimensions { 73, 58 };
    if (version.eq(1, 0, 0)){ 
        {
            H5::H5File fhandle(path, H5F_ACC_TRUNC);
            auto ghandle = constant_array_opener(fhandle, "constant", dimensions, version);
            auto dhandle = add_numeric_scalar(ghandle, "value", 1, H5::PredType::NATIVE_UINT32);
            add_numeric_missing_placeholder(dhandle, 1, H5::PredType::NATIVE_UINT8);
        }
        auto output = test_validate(path, "constant", deets);
        EXPECT_EQ(output.type, chihaya::INTEGER);
        EXPECT_EQ(output.dimensions, dimensions);

    } else if (version.ge(1, 1, 0)) {
        {
            H5::H5File fhandle(path, H5F_ACC_TRUNC);
            auto ghandle = constant_array_opener(fhandle, "constant", dimensions, version);
            auto dhandle = add_numeric_scalar(ghandle, "value", 1, H5::PredType::NATIVE_UINT8);
            add_numeric_missing_placeholder(dhandle, 1, H5::PredType::NATIVE_UINT8);
            add_string_attribute(dhandle, "type", "INTEGER");
        }
        auto output = test_validate(path, "constant", deets);
        EXPECT_EQ(output.type, chihaya::INTEGER);
        EXPECT_EQ(output.dimensions, dimensions);
    }
}

INSTANTIATE_TEST_SUITE_P(
    ConstantArray,
    ConstantArrayPassTest,
    ::testing::Combine(
        spawn_all_versions(),
        ::testing::Values(false, true)
    )
);

/***********************************/

class ConstantArrayErrorTest : public ::testing::TestWithParam<ritsuko::Version> {};

TEST_P(ConstantArrayErrorTest, Dimension) {
    auto path = define_test_path("constant_array");
    auto version = GetParam();

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = constant_array_opener(fhandle, "constant", {}, version);
        ghandle.unlink("dimensions");
        std::vector<hsize_t> dims(2);
        H5::DataSpace dspace(2, dims.data());
        ghandle.createDataSet("dimensions", H5::PredType::NATIVE_UINT32, dspace);
    }
    expect_error(path, "constant", "1-dimensional");

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        constant_array_opener(fhandle, "constant", {}, version);
    }
    expect_error(path, "constant", "should have non-zero");

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = constant_array_opener(fhandle, "constant", { 20, 17 }, version);
        ghandle.unlink("dimensions");
        add_numeric_vector<int>(ghandle, "dimensions", {20, 17}, H5::PredType::NATIVE_DOUBLE);
    }
    if (version.lt(1, 1, 0)) {
        expect_error(path, "constant", "integer type");
    } else {
        expect_error(path, "constant", "64-bit unsigned integer");
    }
}

TEST_P(ConstantArrayErrorTest, Value) {
    auto path = define_test_path("constant_array");
    auto version = GetParam();

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = constant_array_opener(fhandle, "constant", { 50, 10 }, version);
        add_numeric_vector<int>(ghandle, "value", { 20, 20 }, H5::PredType::NATIVE_INT);
    }
    expect_error(path, "constant", "should be a scalar");

    if (version.ge(1, 1, 0)) {
        // Test that we actually check for a scalar 'type'.
        {
            H5::H5File fhandle(path, H5F_ACC_TRUNC);
            auto ghandle = constant_array_opener(fhandle, "constant", { 50, 10 }, version);
            auto dhandle = add_numeric_scalar(ghandle, "value", 0.1, H5::PredType::NATIVE_DOUBLE);
            constexpr hsize_t one = 1;
            dhandle.createAttribute("type", H5::StrType(0, H5T_VARIABLE), H5::DataSpace(1, &one));
        }
        expect_error(path, "constant", "scalar");

        // Test that we actually check the 'type' is consistent with the dataset's type.
        {
            H5::H5File fhandle(path, H5F_ACC_RDWR);
            auto ghandle = fhandle.openGroup("constant");
            auto dhandle = ghandle.openDataSet("value");
            dhandle.removeAttr("type");
            add_string_attribute(dhandle, "type", "INTEGER");
        }
        expect_error(path, "constant", "32-bit signed");
    }

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = constant_array_opener(fhandle, "constant", { 50, 10 }, version);
        H5::StrType stype(0, H5T_VARIABLE);
        auto dhandle = ghandle.createDataSet("value", stype, H5S_SCALAR); 
        if (version.ge(1, 1, 0)) {
            add_string_attribute(dhandle, "type", "STRING");
        }
    }
    expect_error(path, "constant", "NULL pointer");
}

TEST_P(ConstantArrayErrorTest, Missing) {
    auto path = define_test_path("constant_array");
    auto version = GetParam();

    if (version.ge(1, 0, 0)) {
        {
            H5::H5File fhandle(path, H5F_ACC_TRUNC);
            auto ghandle = constant_array_opener(fhandle, "constant", { 20, 17 }, version);
            auto dhandle = add_numeric_scalar(ghandle, "value", 1, H5::PredType::NATIVE_INT);
            if (version.ge(1, 1, 0)) {
                add_string_attribute(dhandle, "type", "INTEGER");
            }
            add_numeric_missing_placeholder(dhandle, 1, H5::PredType::NATIVE_DOUBLE);
        }
        if (version.ge(1, 1, 0)) {
            expect_error(path, "constant", "same datatype as");
        } else {
            expect_error(path, "constant", "same datatype class");
        }
    }
}

INSTANTIATE_TEST_SUITE_P(
    ConstantArray,
    ConstantArrayErrorTest,
    spawn_all_versions()
);
