#include <gtest/gtest.h>

#include <vector>
#include <string>
#include <cstddef>

#include "H5Cpp.h"
#include "ritsuko/ritsuko.hpp"
#include "chihaya/dense_array.hpp"

#include "utils.h"

static H5::Group dense_array_opener(
    H5::Group& handle,
    const std::string& name,
    const std::vector<std::size_t>& dimensions,
    const H5::DataType& type,
    const ritsuko::Version& version,
    bool native
) {
    auto ghandle = array_opener(handle, name, "dense array");
    add_version_string(ghandle, version);

    std::vector<hsize_t> dims(dimensions.begin(), dimensions.end());
    H5::DataSpace dspace(dims.size(), dims.data());
    auto dhandle = ghandle.createDataSet("data", type, dspace);

    if (version.ge(1, 1, 0)) {
        auto cls = type.getClass();
        std::string type;
        if (cls == H5T_FLOAT) {
            type = "FLOAT";
        } else if (cls == H5T_INTEGER) {
            type = "INTEGER";
        } else {
            type = "STRING";
        }
        add_string_attribute(dhandle, "type", type);
    }

    add_numeric_scalar<int>(ghandle, "native", native, H5::PredType::NATIVE_INT8);
    return ghandle;
}

/***********************************/

class DenseArrayPassTest : public ::testing::TestWithParam<std::tuple<ritsuko::Version, bool> > {};

TEST_P(DenseArrayPassTest, Integer) {
    auto path = define_test_path("dense_array");
    auto params = GetParam();
    auto version = std::get<0>(params);
    auto deets = std::get<1>(params);

    std::vector<std::size_t> dims{ 20, 17 };
    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        dense_array_opener(fhandle, "dense", dims, H5::PredType::NATIVE_INT16, version, /* native = */ true); 
    }

    auto output = test_validate(path, "dense", deets); 
    EXPECT_EQ(output.type, chihaya::INTEGER);
    EXPECT_EQ(output.dimensions, dims);
}

TEST_P(DenseArrayPassTest, Float) {
    auto path = define_test_path("dense_array");
    auto params = GetParam();
    auto version = std::get<0>(params);
    auto deets = std::get<1>(params);

    std::vector<std::size_t> dims{ 15 };
    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        dense_array_opener(fhandle, "dense", dims, H5::PredType::NATIVE_FLOAT, version, /* native = */ true); 
    }

    auto output = test_validate(path, "dense", deets);
    EXPECT_EQ(output.type, chihaya::FLOAT);
    EXPECT_EQ(output.dimensions, dims);
}

TEST_P(DenseArrayPassTest, String) {
    auto path = define_test_path("dense_array");
    auto params = GetParam();
    auto version = std::get<0>(params);
    auto deets = std::get<1>(params);

    std::vector<std::size_t> dims{ 3, 5, 4 };
    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        dense_array_opener(fhandle, "dense", dims, H5::StrType(0, 3), version, /* native = */ true); 
    }

    auto output = test_validate(path, "dense", deets);
    EXPECT_EQ(output.type, chihaya::STRING);
    EXPECT_EQ(output.dimensions, dims);
}

TEST_P(DenseArrayPassTest, NonNative) {
    auto path = define_test_path("dense_array");
    auto params = GetParam();
    auto version = std::get<0>(params);
    auto deets = std::get<1>(params);

    std::vector<std::size_t> dims{ 10, 20 };
    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        dense_array_opener(fhandle, "dense", dims, H5::PredType::NATIVE_INT32, version, /* native = */ false); 
    }

    auto output = test_validate(path, "dense", deets);
    EXPECT_EQ(output.type, chihaya::INTEGER);
    auto modified = dims;
    std::reverse(modified.begin(), modified.end());
    EXPECT_EQ(output.dimensions, modified);
}

TEST_P(DenseArrayPassTest, Missing) {
    auto path = define_test_path("dense_array");
    auto params = GetParam();
    auto version = std::get<0>(params);
    auto deets = std::get<1>(params);

    std::vector<std::size_t> dimensions{ 42, 21 };
    if (version.eq(1, 0, 0)) {
        {
            H5::H5File fhandle(path, H5F_ACC_TRUNC);
            auto ghandle = dense_array_opener(fhandle, "dense", dimensions, H5::PredType::NATIVE_INT32, version, /* native = */ true); 
            auto dhandle = ghandle.openDataSet("data");
            add_numeric_missing_placeholder<int>(dhandle, 2, H5::PredType::NATIVE_INT8);
        }
        auto output = test_validate(path, "dense", deets);
        EXPECT_EQ(output.type, chihaya::INTEGER);
        EXPECT_EQ(output.dimensions, dimensions);

    } else if (version.ge(1, 1, 0)) {
        {
            H5::H5File fhandle(path, H5F_ACC_TRUNC);
            auto ghandle = dense_array_opener(fhandle, "dense", dimensions, H5::PredType::NATIVE_INT32, version, /* native = */ true); 
            auto dhandle = ghandle.openDataSet("data");
            add_numeric_missing_placeholder<int>(dhandle, 2, H5::PredType::NATIVE_INT32);
        }
        auto output = test_validate(path, "dense", deets); 
        EXPECT_EQ(output.type, chihaya::INTEGER);
        EXPECT_EQ(output.dimensions, dimensions);
    }
}

TEST_P(DenseArrayPassTest, Dimnames) {
    auto path = define_test_path("dense_array");
    auto params = GetParam();
    auto version = std::get<0>(params);
    auto deets = std::get<1>(params);

    std::vector<std::size_t> dims{ 19, 82 };
    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = dense_array_opener(fhandle, "dense", dims, H5::PredType::NATIVE_INT32, version, /* native = */ true); 
        auto lhandle = list_opener(ghandle, "dimnames", 2, version);
        add_string_vector(lhandle, "0", dims[0], /* len = */ 2);
        add_string_vector(lhandle, "1", dims[1], /* len = */ 2);
    }
    {
        auto output = test_validate(path, "dense", deets); 
        EXPECT_EQ(output.type, chihaya::INTEGER);
        EXPECT_EQ(output.dimensions, dims);
    }

    // Works correctly in native mode with shuffling of dimensions.
    {
        H5::H5File fhandle(path, H5F_ACC_RDWR);
        auto ghandle = fhandle.openGroup("dense");
        ghandle.unlink("native");
        add_numeric_scalar(ghandle, "native", 0, H5::PredType::NATIVE_INT8); 
    }
    {
        auto output = test_validate(path, "dense", deets); 
        EXPECT_EQ(output.type, chihaya::INTEGER);
        auto modified = dims;
        std::reverse(modified.begin(), modified.end());
        EXPECT_EQ(output.dimensions, modified);
    }
}

TEST_P(DenseArrayPassTest, Boolean) {
    auto path = define_test_path("dense_array");
    auto params = GetParam();
    auto version = std::get<0>(params);
    auto deets = std::get<1>(params);

    std::vector<std::size_t> dimensions{ 13, 12, 4 };
    if (version.lt(1, 1, 0)) {
        {
            H5::H5File fhandle(path, H5F_ACC_TRUNC);
            auto ghandle = dense_array_opener(fhandle, "dense", dimensions, H5::PredType::NATIVE_INT, version, /* native = */ true); 
            auto dhandle = ghandle.openDataSet("data");
            auto ahandle = dhandle.createAttribute("is_boolean", H5::PredType::NATIVE_INT, H5S_SCALAR);
            int val = 1;
            ahandle.write(H5::PredType::NATIVE_INT, &val);
        }
    } else {
        {
            H5::H5File fhandle(path, H5F_ACC_TRUNC);
            auto ghandle = dense_array_opener(fhandle, "dense", dimensions, H5::PredType::NATIVE_INT8, version, /* native = */ true); 
            auto dhandle = ghandle.openDataSet("data");
            dhandle.removeAttr("type");
            add_string_attribute(dhandle, "type", "BOOLEAN");
        }
    }

    auto output = test_validate(path, "dense", deets); 
    EXPECT_EQ(output.type, chihaya::BOOLEAN);
    EXPECT_EQ(output.dimensions, dimensions);
}

INSTANTIATE_TEST_SUITE_P(
    DenseArray,
    DenseArrayPassTest,
    ::testing::Combine(
        spawn_all_versions(),
        ::testing::Values(false, true)
    )
);

/***********************************/

class DenseArrayErrorTest : public ::testing::TestWithParam<ritsuko::Version> {};

TEST_P(DenseArrayErrorTest, Data) {
    auto path = define_test_path("dense_array");
    auto version = GetParam();

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = dense_array_opener(fhandle, "dense", { 20, 17 }, H5::PredType::NATIVE_FLOAT, version, /* native = */ true);
        ghandle.unlink("data");
        add_numeric_scalar<int>(ghandle, "data", 50, H5::PredType::NATIVE_INT32);
    }
    expect_error(path, "dense", "non-zero dimensions");

    if (version.ge(1, 1, 0)) {
        // Test that we actually check for a scalar 'type'.
        {
            H5::H5File fhandle(path, H5F_ACC_TRUNC);
            auto ghandle = dense_array_opener(fhandle, "dense", { 20, 17 }, H5::PredType::NATIVE_FLOAT, version, /* native = */ true);
            auto dhandle = ghandle.openDataSet("data");
            dhandle.removeAttr("type");
            constexpr hsize_t one = 1;
            dhandle.createAttribute("type", H5::StrType(0, H5T_VARIABLE), H5::DataSpace(1, &one));
        }
        expect_error(path, "dense", "scalar");

        // Test that we actually check the 'type' is consistent with the dataset's type.
        {
            H5::H5File fhandle(path, H5F_ACC_RDWR);
            auto ghandle = fhandle.openGroup("dense");
            auto dhandle = ghandle.openDataSet("data");
            dhandle.removeAttr("type");
            add_string_attribute(dhandle, "type", "INTEGER");
        }
        expect_error(path, "dense", "32-bit signed integer");
    }
}

TEST_P(DenseArrayErrorTest, Missing) {
    auto path = define_test_path("dense_array");
    auto version = GetParam();

    if (version.ge(1, 0, 0)) {
        {
            H5::H5File fhandle(path, H5F_ACC_TRUNC);
            auto ghandle = dense_array_opener(fhandle, "dense", { 20, 17 }, H5::PredType::NATIVE_INT, version, /* native = */ true); 
            auto dhandle = ghandle.openDataSet("data");
            add_numeric_missing_placeholder(dhandle, 1, H5::PredType::NATIVE_DOUBLE);
        }
        if (version.lt(1, 1, 0)) {
            expect_error(path, "dense", "same datatype class");
        } else {
            expect_error(path, "dense", "same datatype as");
        }
    }
}

TEST_P(DenseArrayErrorTest, NullStrings) {
    auto path = define_test_path("dense_array");
    auto version = GetParam();

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = dense_array_opener(fhandle, "dense", { 20, 17 }, H5::PredType::NATIVE_FLOAT, version, /* native = */ true);
        ghandle.unlink("data");
        H5::StrType stype(0, H5T_VARIABLE);
        hsize_t dim[2] = { 10, 20 };
        H5::DataSpace dspace(2, dim);
        auto dhandle = ghandle.createDataSet("data", stype, dspace);
        if (version.ge(1, 1, 0)) {
            add_string_attribute(dhandle, "type", "STRING");
        }
    }
    expect_error(path, "dense", "NULL");
}

TEST_P(DenseArrayErrorTest, Native) {
    auto path = define_test_path("dense_array");
    auto version = GetParam();

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = dense_array_opener(fhandle, "dense", { 20, 17 }, H5::PredType::NATIVE_FLOAT, version, /* native = */ true);
        ghandle.unlink("native");
        add_numeric_vector<int>(ghandle, "native", { 2 }, H5::PredType::NATIVE_INT8);
    }
    expect_error(path, "dense", "should be a scalar");

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = dense_array_opener(fhandle, "dense", { 20, 17 }, H5::PredType::NATIVE_FLOAT, version, /* native = */ true);
        ghandle.unlink("native");
        add_numeric_scalar<int>(ghandle, "native", 2, H5::PredType::NATIVE_FLOAT);
    }
    if (version.lt(1, 1, 0)) {
        expect_error(path, "dense", "expected an integer type");
    } else {
        expect_error(path, "dense", "8-bit signed integer");
    }
}

TEST_P(DenseArrayErrorTest, Dimnames) {
    auto path = define_test_path("dense_array");
    auto version = GetParam();

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = dense_array_opener(fhandle, "dense", { 50, 10 }, H5::PredType::NATIVE_INT, version, /* native = */ true); 
        auto lhandle = list_opener(ghandle, "dimnames", 2, version);
        mock_array_opener<int>(lhandle, "3", { 2 }, version, "STRING");
    }
    expect_error(path, "dense", "dimnames");
}

TEST_P(DenseArrayErrorTest, Boolean) {
    auto path = define_test_path("dense_array");
    auto version = GetParam();

    if (version.lt(1, 1, 0)) {
        {
            H5::H5File fhandle(path, H5F_ACC_TRUNC);
            auto ghandle = dense_array_opener(fhandle, "dense", { 50, 10 }, H5::PredType::NATIVE_INT, version, /* native = */ true); 
            auto dhandle = ghandle.openDataSet("data");
            add_string_attribute(dhandle, "is_boolean", "YAY");
        }
        expect_error(path, "dense", "expected an integer type");

    } else {
        {
            H5::H5File fhandle(path, H5F_ACC_TRUNC);
            auto ghandle = dense_array_opener(fhandle, "dense", { 50, 10 }, H5::PredType::NATIVE_INT32, version, /* native = */ true); 
            auto dhandle = ghandle.openDataSet("data");
            dhandle.removeAttr("type");
            add_string_attribute(dhandle, "type", "BOOLEAN");
        }
        expect_error(path, "dense", "8-bit signed integer");
    }
}

INSTANTIATE_TEST_SUITE_P(
    DenseArray,
    DenseArrayErrorTest,
    spawn_all_versions()
);
