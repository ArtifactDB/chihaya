#include <gtest/gtest.h>

#include <string>
#include <vector>
#include <cstddef>

#include "H5Cpp.h"
#include "ritsuko/ritsuko.hpp"
#include "chihaya/sparse_matrix.hpp"

#include "utils.h"

class SparseMatrixCore {
public:
    inline static std::string path;
    inline static std::vector<std::size_t> dims;
    inline static std::vector<double> data;
    inline static std::vector<int> indices;
    inline static std::vector<int> indptr;

    static void initialize() {
        path = define_test_path("sparse_matrix");
        dims = std::vector<std::size_t>{ 10, 5 };
        data = std::vector<double>{ -1.10, 0.18, 0.95, -0.17, -0.031, -0.75, 0.13, -0.89, 0.74, -0.43 };
        indices = std::vector<int>{ 0, 4, 4, 5, 7, 8, 4, 9, 2, 9 };
        indptr = std::vector<int>{ 0, 2, 5, 6, 8, 10 };
    }

    static H5::Group sparse_matrix_opener(H5::Group& handle, const ritsuko::Version& version) {
        auto ghandle = array_opener(handle, "foobar", "sparse matrix");
        add_version_string(ghandle, version);

        auto dhandle = add_numeric_vector(ghandle, "data", data, H5::PredType::NATIVE_DOUBLE);
        if (version.lt(1, 1, 0)) {
            add_numeric_vector(ghandle, "shape", dims, H5::PredType::NATIVE_INT);
            add_numeric_vector(ghandle, "indices", indices, H5::PredType::NATIVE_INT);
            add_numeric_vector(ghandle, "indptr", indptr, H5::PredType::NATIVE_INT);
        } else {
            add_numeric_vector(ghandle, "shape", dims, H5::PredType::NATIVE_UINT32);
            add_numeric_vector(ghandle, "indices", indices, H5::PredType::NATIVE_UINT32);
            add_numeric_vector(ghandle, "indptr", indptr, H5::PredType::NATIVE_UINT64);
            add_string_attribute(dhandle, "type", "FLOAT");
            add_numeric_scalar(ghandle, "by_column", 1, H5::PredType::NATIVE_INT8);
        }

        return ghandle;
    } 
};

/***************************************/

class SparseMatrixPassTest : public ::testing::TestWithParam<std::tuple<ritsuko::Version, bool> >, public SparseMatrixCore {
protected:
    static void SetUpTestSuite() {
        initialize();
    }
};

TEST_P(SparseMatrixPassTest, Basic) {
    auto params = GetParam();
    auto version = std::get<0>(params);
    auto deets = std::get<1>(params);

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        sparse_matrix_opener(fhandle, version);
    }

    auto output = test_validate(path, "foobar", deets); 
    EXPECT_EQ(output.type, chihaya::FLOAT);
    EXPECT_EQ(output.dimensions, dims);
}

TEST_P(SparseMatrixPassTest, Csr) {
    auto params = GetParam();
    auto version = std::get<0>(params);
    auto deets = std::get<1>(params);

    if (version.lt(1, 1, 0)) {
        return;
    }

    // Works for CSR if we flip the rows and columns.
    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = array_opener(fhandle, "foobar", "sparse matrix");
        add_version_string(ghandle, version);

        auto dhandle = add_numeric_vector(ghandle, "data", data, H5::PredType::NATIVE_DOUBLE);
        add_numeric_vector<std::size_t>(ghandle, "shape", { dims[1], dims[0]}, H5::PredType::NATIVE_UINT32);
        add_numeric_vector(ghandle, "indices", indices, H5::PredType::NATIVE_UINT32);
        add_numeric_vector(ghandle, "indptr", indptr, H5::PredType::NATIVE_UINT64);
        add_string_attribute(dhandle, "type", "FLOAT");
        add_numeric_scalar(ghandle, "by_column", 0, H5::PredType::NATIVE_INT8);
    }

    auto output = test_validate(path, "foobar", deets); 
    EXPECT_EQ(output.type, chihaya::FLOAT);
    EXPECT_EQ(output.dimensions.size(), 2);
    EXPECT_EQ(output.dimensions[0], dims[1]);
    EXPECT_EQ(output.dimensions[1], dims[0]);
}

TEST_P(SparseMatrixPassTest, Dimnames) {
    auto params = GetParam();
    auto version = std::get<0>(params);
    auto deets = std::get<1>(params);

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = sparse_matrix_opener(fhandle, version);
        auto lhandle = list_opener(ghandle, "dimnames", 2, version);
        add_string_vector(lhandle, "0", dims[0], /* len = */ 2);
        add_string_vector(lhandle, "1", dims[1], /* len = */ 2);
    }

    auto output = test_validate(path, "foobar", deets); 
    EXPECT_EQ(output.type, chihaya::FLOAT);
    EXPECT_EQ(output.dimensions, dims);
}

TEST_P(SparseMatrixPassTest, Boolean) {
    auto params = GetParam();
    auto version = std::get<0>(params);
    auto deets = std::get<1>(params);

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = sparse_matrix_opener(fhandle, version);
        ghandle.unlink("data");

        std::vector<int> copy(data.begin(), data.end());
        if (version.lt(1, 1, 0)) {
            auto dhandle = add_numeric_vector(ghandle, "data", copy, H5::PredType::NATIVE_INT); 
            auto ahandle = dhandle.createAttribute("is_boolean", H5::PredType::NATIVE_INT, H5S_SCALAR);
            int val = 1;
            ahandle.write(H5::PredType::NATIVE_INT, &val);
        } else {
            auto dhandle = add_numeric_vector(ghandle, "data", copy, H5::PredType::NATIVE_INT8); 
            add_string_attribute(dhandle, "type", "BOOLEAN");
        }
    }

    auto output = test_validate(path, "foobar", deets); 
    EXPECT_EQ(output.type, chihaya::BOOLEAN);
    EXPECT_EQ(output.dimensions, dims);
}

TEST_P(SparseMatrixPassTest, Missing) {
    auto params = GetParam();
    auto version = std::get<0>(params);
    auto deets = std::get<1>(params);

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = sparse_matrix_opener(fhandle, version);
        auto dhandle = ghandle.openDataSet("data");
        add_numeric_missing_placeholder(dhandle, 2.5, H5::PredType::NATIVE_DOUBLE);
    }

    auto output = test_validate(path, "foobar", deets); 
    EXPECT_EQ(output.type, chihaya::FLOAT);
    EXPECT_EQ(output.dimensions, dims);
}

INSTANTIATE_TEST_SUITE_P(
    SparseMatrix,
    SparseMatrixPassTest,
    ::testing::Combine(
        spawn_all_versions(),
        ::testing::Values(false, true)
    )
);

/***************************************/

class SparseMatrixErrorTest : public ::testing::TestWithParam<ritsuko::Version>, public SparseMatrixCore {
protected:
    static void SetUpTestSuite() {
        initialize();
    }
};

TEST_P(SparseMatrixErrorTest, Shape) {
    auto version = GetParam();

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = array_opener(fhandle, "foobar", "sparse matrix");
        add_version_string(ghandle, version);
        add_numeric_scalar(ghandle, "shape", 10, H5::PredType::NATIVE_UINT8);
    }
    expect_error(path, "foobar", "1-dimensional");

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = array_opener(fhandle, "foobar", "sparse matrix");
        add_version_string(ghandle, version);
        add_numeric_vector<int>(ghandle, "shape", { 10, 5, 2 }, H5::PredType::NATIVE_UINT8);
    }
    expect_error(path, "foobar", "'shape' dataset should have length 2");

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = array_opener(fhandle, "foobar", "sparse matrix");
        add_version_string(ghandle, version);
        add_numeric_vector(ghandle, "shape", dims, H5::PredType::NATIVE_DOUBLE);
    }
    if (version.lt(1, 1, 0)) {
        expect_error(path, "foobar", "expected an integer type");
    } else {
        expect_error(path, "foobar", "64-bit unsigned integer");
    }

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = array_opener(fhandle, "foobar", "sparse matrix");
        add_version_string(ghandle, version);
        add_numeric_vector<int>(ghandle, "shape", { -1, 10 }, H5::PredType::NATIVE_INT);
    }
    if (version.lt(1, 1, 0)) {
        expect_error(path, "foobar", "should contain non-negative");
    } else {
        expect_error(path, "foobar", "64-bit unsigned integer");
    }
}

TEST_P(SparseMatrixErrorTest, Data) {
    auto version = GetParam();

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = sparse_matrix_opener(fhandle, version);
        ghandle.unlink("data");
        ghandle.createDataSet("data", H5::PredType::NATIVE_INT, H5S_SCALAR);
    }
    expect_error(path, "foobar", "should be 1-dimensional");

    {
        H5::H5File fhandle(path, H5F_ACC_RDWR);
        auto ghandle = fhandle.openGroup("foobar");
        ghandle.unlink("data");

        auto dhandle = add_string_vector(ghandle, "data", 20);
        if (version.ge(1, 1, 0)) {
            add_string_attribute(dhandle, "type", "STRING");
        }
    }
    expect_error(path, "foobar", "integer, float or boolean");
}

TEST_P(SparseMatrixErrorTest, ByColumn) {
    auto version = GetParam();
    if (version.lt(1, 1, 0)) {
        return;
    }

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = sparse_matrix_opener(fhandle, version);
        ghandle.unlink("by_column");
        constexpr hsize_t one = 1;
        ghandle.createDataSet("by_column", H5::PredType::NATIVE_INT8, H5::DataSpace(1, &one));
    }
    expect_error(path, "foobar", "scalar");

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = sparse_matrix_opener(fhandle, version);
        ghandle.unlink("by_column");
        ghandle.createDataSet("by_column", H5::PredType::NATIVE_DOUBLE, H5S_SCALAR);
    }
    expect_error(path, "foobar", "8-bit signed integer");
}

TEST_P(SparseMatrixErrorTest, Boolean) {
    auto version = GetParam();
    if (version.ge(1, 1, 0)) {
        return;
    }

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = sparse_matrix_opener(fhandle, version);
        ghandle.unlink("data");
        std::vector<int> copy(data.begin(), data.end());
        auto dhandle = add_numeric_vector(ghandle, "data", copy, H5::PredType::NATIVE_INT); 
        auto ahandle = dhandle.createAttribute("is_boolean", H5::PredType::NATIVE_DOUBLE, H5S_SCALAR);
    }
    expect_error(path, "foobar", "expected an integer");
}

TEST_P(SparseMatrixErrorTest, SimpleIndex) {
    auto version = GetParam();

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = sparse_matrix_opener(fhandle, version);
        ghandle.unlink("indices");
        hsize_t dims[2] = { 10, 20 };
        ghandle.createDataSet("indices", H5::PredType::NATIVE_UINT8, H5::DataSpace(2, dims));
    }
    expect_error(path, "foobar", "1-dimensional");

    {
        H5::H5File fhandle(path, H5F_ACC_RDWR);
        auto ghandle = fhandle.openGroup("foobar");
        ghandle.unlink("indices");
        add_numeric_vector(ghandle, "indices", indices, H5::PredType::NATIVE_DOUBLE);
    }
    if (version.lt(1, 1, 0)) {
        expect_error(path, "foobar", "'indices' should be integer");
    } else {
        expect_error(path, "foobar", "64-bit unsigned integer");
    }

    {
        H5::H5File fhandle(path, H5F_ACC_RDWR);
        auto ghandle = fhandle.openGroup("foobar");
        ghandle.unlink("indices");
        add_numeric_vector<int>(ghandle, "indices", { 1, 2 }, H5::PredType::NATIVE_UINT32);
    }
    expect_error(path, "foobar", "same length");
}

TEST_P(SparseMatrixErrorTest, Indptr) {
    auto version = GetParam();

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = sparse_matrix_opener(fhandle, version);
        ghandle.unlink("indptr");
        add_numeric_scalar(ghandle, "indptr", 5, H5::PredType::NATIVE_UINT8);
    }
    expect_error(path, "foobar", "1-dimensional");

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = sparse_matrix_opener(fhandle, version);
        ghandle.unlink("indptr");
        add_numeric_vector(ghandle, "indptr", indptr, H5::PredType::NATIVE_DOUBLE);
    }
    if (version.lt(1, 1, 0)) {
        expect_error(path, "foobar", "'indptr' should be integer");
    } else {
        expect_error(path, "foobar", "64-bit unsigned integer");
    }

    {
        H5::H5File fhandle(path, H5F_ACC_RDWR);
        auto ghandle = fhandle.openGroup("foobar");
        ghandle.unlink("indptr");
        add_numeric_vector<int>(ghandle, "indptr", { 0 }, H5::PredType::NATIVE_UINT32);
    }
    expect_error(path, "foobar", "'indptr' should have length");

    {
        H5::H5File fhandle(path, H5F_ACC_RDWR);
        auto ghandle = fhandle.openGroup("foobar");
        ghandle.unlink("indptr");
        auto copy = indptr;
        copy[0] = 1;
        add_numeric_vector<int>(ghandle, "indptr", copy, H5::PredType::NATIVE_UINT32);
    }
    expect_error(path, "foobar", "first entry");

    {
        H5::H5File fhandle(path, H5F_ACC_RDWR);
        auto ghandle = fhandle.openGroup("foobar");
        ghandle.unlink("indptr");
        auto copy = indptr;
        copy.back() = 1;
        add_numeric_vector<int>(ghandle, "indptr", copy, H5::PredType::NATIVE_UINT32);
    }
    expect_error(path, "foobar", "last entry");

    {
        H5::H5File fhandle(path, H5F_ACC_RDWR);
        auto ghandle = fhandle.openGroup("foobar");
        ghandle.unlink("indptr");
        auto copy = indptr;
        copy[2] = copy[1] - 1;
        add_numeric_vector<int>(ghandle, "indptr", copy, H5::PredType::NATIVE_UINT32);
    }
    expect_error(path, "foobar", "sorted");
}

TEST_P(SparseMatrixErrorTest, ComplexIndex) {
    auto version = GetParam();

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = sparse_matrix_opener(fhandle, version);
        ghandle.unlink("indices");
        auto copy = indices;
        copy.back() = dims[1] + 10;
        add_numeric_vector<int>(ghandle, "indices", copy, H5::PredType::NATIVE_UINT16);
    }
    expect_error(path, "foobar", "number of rows");

    if (version.lt(1, 1, 0)) {
        {
            H5::H5File fhandle(path, H5F_ACC_RDWR);
            auto ghandle = fhandle.openGroup("foobar");
            ghandle.unlink("indices");
            auto copy = indices;
            copy[0] = -1;
            add_numeric_vector<int>(ghandle, "indices", copy, H5::PredType::NATIVE_INT);
        }
        expect_error(path, "foobar", "non-negative");
    }

    {
        H5::H5File fhandle(path, H5F_ACC_RDWR);
        auto ghandle = fhandle.openGroup("foobar");
        ghandle.unlink("indices");
        auto copy = indices;
        std::fill(copy.begin(), copy.end(), 0);
        add_numeric_vector<int>(ghandle, "indices", copy, H5::PredType::NATIVE_UINT16);
    }
    expect_error(path, "foobar", "strictly increasing");
}

TEST_P(SparseMatrixErrorTest, Missing) {
    auto version = GetParam();

    if (version.ge(1, 0, 0)) {
        {
            H5::H5File fhandle(path, H5F_ACC_TRUNC);
            auto ghandle = sparse_matrix_opener(fhandle, version);
            auto dhandle = ghandle.openDataSet("data");
            add_numeric_missing_placeholder(dhandle, 1, H5::PredType::NATIVE_INT32);
        }
        if (version.lt(1, 1, 0)) {
            expect_error(path, "foobar", "same datatype class");
        } else {
            expect_error(path, "foobar", "same datatype as");
        }
    }
}

TEST_P(SparseMatrixErrorTest, Dimnames) {
    auto version = GetParam();

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = sparse_matrix_opener(fhandle, version);
        auto lhandle = list_opener(ghandle, "dimnames", 2, version);
        add_string_vector(lhandle, "0", 99, /* len = */ 2);
    }
    expect_error(path, "foobar", "length equal to the extent");
}

INSTANTIATE_TEST_SUITE_P(
    SparseMatrix,
    SparseMatrixErrorTest,
    spawn_all_versions()
);
