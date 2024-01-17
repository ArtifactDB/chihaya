#include <gtest/gtest.h>
#include "chihaya/chihaya.hpp"
#include "utils.h"

class DenseArrayTest : public ::testing::TestWithParam<int> {
public:
    DenseArrayTest() : path("Test_dense_array.h5") {}

protected:
    std::string path;

    H5::Group dense_array_opener(H5::Group& handle, const std::string& name, std::vector<hsize_t> dimensions, const H5::DataType& type, int version, bool native = true) {
        auto ghandle = array_opener(handle, name, "dense array");
        add_version_string(ghandle, version);

        if (!native) {
            std::reverse(dimensions.begin(), dimensions.end());
        }
        H5::DataSpace dspace(dimensions.size(), dimensions.data());
        auto dhandle = ghandle.createDataSet("data", type, dspace);

        if (version >= 1100000) {
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
};

TEST_P(DenseArrayTest, Basic) {
    auto version = GetParam();

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        dense_array_opener(fhandle, "dense", { 20, 17 }, H5::PredType::NATIVE_INT16, version); 
    }
    {
        auto output = test_validate(path, "dense"); 
        EXPECT_EQ(output.type, chihaya::INTEGER);
        const auto& dims = output.dimensions;
        EXPECT_EQ(dims.size(), 2);
        EXPECT_EQ(dims[0], 20);
        EXPECT_EQ(dims[1], 17);

        auto skipped = test_validate_skip(path, "dense");
        EXPECT_EQ(skipped.type, output.type);
        EXPECT_EQ(skipped.dimensions, output.dimensions);
    }

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        dense_array_opener(fhandle, "dense", { 5, 17 }, H5::PredType::NATIVE_FLOAT, version); 
    }
    {
        auto output = test_validate(path, "dense"); 
        EXPECT_EQ(output.type, chihaya::FLOAT);
        const auto& dims = output.dimensions;
        EXPECT_EQ(dims.size(), 2);
        EXPECT_EQ(dims[0], 5);
        EXPECT_EQ(dims[1], 17);
    }

    // Get some coverage for string types.
    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        dense_array_opener(fhandle, "dense", { 3, 4, 5 }, H5::StrType(0, 3), version); 
    }
    {
        auto output = test_validate(path, "dense"); 
        EXPECT_EQ(output.type, chihaya::STRING);
        const auto& dims = output.dimensions;
        EXPECT_EQ(dims.size(), 3);
        EXPECT_EQ(dims[0], 3);
        EXPECT_EQ(dims[1], 4);
        EXPECT_EQ(dims[2], 5);
    }
}

TEST_P(DenseArrayTest, NonNative) {
    auto version = GetParam();

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        dense_array_opener(fhandle, "dense", { 20, 17 }, H5::PredType::NATIVE_INT32, version, false); 
    }
    {
        auto output = test_validate(path, "dense"); 
        EXPECT_EQ(output.type, chihaya::INTEGER);
        const auto& dims = output.dimensions;
        EXPECT_EQ(dims.size(), 2);
        EXPECT_EQ(dims[0], 20);
        EXPECT_EQ(dims[1], 17);
    }
}

TEST_P(DenseArrayTest, Missing) {
    auto version = GetParam();

    if (version == 1000000) {
        {
            H5::H5File fhandle(path, H5F_ACC_TRUNC);
            auto ghandle = dense_array_opener(fhandle, "dense", { 20, 17 }, H5::PredType::NATIVE_INT32, version, false); 
            auto dhandle = ghandle.openDataSet("data");
            add_numeric_missing_placeholder<int>(dhandle, 2, H5::PredType::NATIVE_INT8);
        }
        auto output = test_validate(path, "dense"); 
        EXPECT_EQ(output.type, chihaya::INTEGER);
    } else if (version >= 1100000) {
        {
            H5::H5File fhandle(path, H5F_ACC_TRUNC);
            auto ghandle = dense_array_opener(fhandle, "dense", { 20, 17 }, H5::PredType::NATIVE_INT32, version, false); 
            auto dhandle = ghandle.openDataSet("data");
            add_numeric_missing_placeholder<int>(dhandle, 2, H5::PredType::NATIVE_INT32);
        }
        auto output = test_validate(path, "dense"); 
        EXPECT_EQ(output.type, chihaya::INTEGER);
    }

    if (version >= 1000000) {
        {
            H5::H5File fhandle(path, H5F_ACC_TRUNC);
            auto ghandle = dense_array_opener(fhandle, "dense", { 20, 17 }, H5::StrType(0, 2), version, false); 
            auto dhandle = ghandle.openDataSet("data");
            add_string_missing_placeholder(dhandle, "foo", H5T_VARIABLE);
        }
        auto output = test_validate(path, "dense"); 
        EXPECT_EQ(output.type, chihaya::STRING);
    }
}

TEST_P(DenseArrayTest, Dimnames) {
    auto version = GetParam();

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = dense_array_opener(fhandle, "dense", { 20, 17 }, H5::PredType::NATIVE_INT32, version); 
        auto lhandle = list_opener(ghandle, "dimnames", 2, version);
        add_string_vector(lhandle, "0", 20, /* len = */ 2);
        add_string_vector(lhandle, "1", 17, /* len = */ 2);
    }
    {
        auto output = test_validate(path, "dense"); 
        EXPECT_EQ(output.type, chihaya::INTEGER);
        EXPECT_EQ(output.dimensions[0], 20);
        EXPECT_EQ(output.dimensions[1], 17);
    }

    // Works correctly in native mode with shuffling of dimensions.
    {
        H5::H5File fhandle(path, H5F_ACC_RDWR);
        auto ghandle = fhandle.openGroup("dense");
        ghandle.unlink("native");
        add_numeric_scalar(ghandle, "native", 0, H5::PredType::NATIVE_INT8); 
    }
    {
        auto output = test_validate(path, "dense"); 
        EXPECT_EQ(output.type, chihaya::INTEGER);
        EXPECT_EQ(output.dimensions[0], 17);
        EXPECT_EQ(output.dimensions[1], 20);
    }
}

TEST_P(DenseArrayTest, Boolean) {
    auto version = GetParam();

    if (version < 1100000) {
        {
            H5::H5File fhandle(path, H5F_ACC_TRUNC);
            auto ghandle = dense_array_opener(fhandle, "dense", { 20, 17 }, H5::PredType::NATIVE_INT, version); 
            auto dhandle = ghandle.openDataSet("data");
            auto ahandle = dhandle.createAttribute("is_boolean", H5::PredType::NATIVE_INT, H5S_SCALAR);
            int val = 1;
            ahandle.write(H5::PredType::NATIVE_INT, &val);
        }
    } else {
        {
            H5::H5File fhandle(path, H5F_ACC_TRUNC);
            auto ghandle = dense_array_opener(fhandle, "dense", { 20, 17 }, H5::PredType::NATIVE_INT8, version); 
            auto dhandle = ghandle.openDataSet("data");
            dhandle.removeAttr("type");
            add_string_attribute(dhandle, "type", "BOOLEAN");
        }
    }

    auto output = test_validate(path, "dense"); 
    EXPECT_EQ(output.type, chihaya::BOOLEAN);

    auto skipped = test_validate_skip(path, "dense");
    EXPECT_EQ(skipped.type, output.type);
    EXPECT_EQ(skipped.dimensions, output.dimensions);
}

TEST_P(DenseArrayTest, DataErrors) {
    auto version = GetParam();

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = dense_array_opener(fhandle, "dense", { 20, 17 }, H5::PredType::NATIVE_FLOAT, version);
        ghandle.unlink("data");
    }
    expect_error(path, "dense", "expected a dataset at 'data'");

    {
        H5::H5File fhandle(path, H5F_ACC_RDWR);
        auto ghandle = fhandle.openGroup("dense");
        add_numeric_scalar<int>(ghandle, "data", 50, H5::PredType::NATIVE_INT32);
    }
    expect_error(path, "dense", "'data' should have non-zero");

    {
        H5::H5File fhandle(path, H5F_ACC_RDWR);
        auto ghandle = fhandle.openGroup("dense");
        ghandle.unlink("data");
        H5::StrType stype(0, H5T_VARIABLE);
        hsize_t dim[2] = { 10, 20 };
        H5::DataSpace dspace(1, dim);
        auto dhandle = ghandle.createDataSet("data", stype, dspace);
        if (version >= 1100000) {
            add_string_attribute(dhandle, "type", "STRING");
        }
    }
    expect_error(path, "dense", "NULL");
}

TEST_P(DenseArrayTest, NativeErrors) {
    auto version = GetParam();

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = dense_array_opener(fhandle, "dense", { 20, 17 }, H5::PredType::NATIVE_FLOAT, version);
        ghandle.unlink("native");
        ghandle.createGroup("native");
    }
    expect_error(path, "dense", "expected a dataset at 'native'");

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = dense_array_opener(fhandle, "dense", { 20, 17 }, H5::PredType::NATIVE_FLOAT, version);
        ghandle.unlink("native");
        add_numeric_vector<int>(ghandle, "native", { 2 }, H5::PredType::NATIVE_INT8);
    }
    expect_error(path, "dense", "should be a scalar");

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = dense_array_opener(fhandle, "dense", { 20, 17 }, H5::PredType::NATIVE_FLOAT, version);
        ghandle.unlink("native");
        add_numeric_scalar<int>(ghandle, "native", 2, H5::PredType::NATIVE_FLOAT);
    }
    if (version < 1100000) {
        expect_error(path, "dense", "should have an integer datatype");
    } else {
        expect_error(path, "dense", "8-bit signed integer");
    }
}

TEST_P(DenseArrayTest, DimnameErrors) {
    auto version = GetParam();

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = dense_array_opener(fhandle, "dense", { 50, 10 }, H5::PredType::NATIVE_INT, version); 
        auto lhandle = list_opener(ghandle, "dimnames", 2);
        lhandle.createGroup("0");
    }
    expect_error(path, "dense", "dimnames");
}

TEST_P(DenseArrayTest, BooleanErrors) {
    auto version = GetParam();

    if (version < 1100000) {
        {
            H5::H5File fhandle(path, H5F_ACC_TRUNC);
            auto ghandle = dense_array_opener(fhandle, "dense", { 50, 10 }, H5::PredType::NATIVE_INT, version); 
            auto dhandle = ghandle.openDataSet("data");
            add_string_attribute(dhandle, "is_boolean", "YAY");
        }
        expect_error(path, "dense", "should be integer");
    } else {
        {
            H5::H5File fhandle(path, H5F_ACC_TRUNC);
            auto ghandle = dense_array_opener(fhandle, "dense", { 50, 10 }, H5::PredType::NATIVE_INT32, version); 
            auto dhandle = ghandle.openDataSet("data");
            dhandle.removeAttr("type");
            add_string_attribute(dhandle, "type", "BOOLEAN");
        }
        expect_error(path, "dense", "8-bit signed integer");
    }
}

TEST_P(DenseArrayTest, MissingErrors) {
    auto version = GetParam();

    if (version >= 1000000) {
        {
            H5::H5File fhandle(path, H5F_ACC_TRUNC);
            auto ghandle = dense_array_opener(fhandle, "dense", { 20, 17 }, H5::PredType::NATIVE_INT, version); 
            auto dhandle = ghandle.openDataSet("data");
            add_numeric_missing_placeholder(dhandle, 1, H5::PredType::NATIVE_DOUBLE);
        }
        if (version < 1100000) {
            expect_error(path, "dense", "have the same type class");
        } else {
            expect_error(path, "dense", "have the same type as");
        }
    }

    if (version >= 1100000) {
        {
            H5::H5File fhandle(path, H5F_ACC_TRUNC);
            auto ghandle = dense_array_opener(fhandle, "dense", { 20, 17 }, H5::PredType::NATIVE_INT32, version);
            auto dhandle = ghandle.openDataSet("data");
            add_numeric_missing_placeholder(dhandle, 1, H5::PredType::NATIVE_INT8);
        }
        expect_error(path, "dense", "same type as");
    }
}

INSTANTIATE_TEST_SUITE_P(
    DenseArray,
    DenseArrayTest,
    ::testing::Values(0, 1000000, 1100000)
);
