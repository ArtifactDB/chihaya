#include <gtest/gtest.h>
#include "chihaya/chihaya.hpp"
#include "utils.h"

class SparseMatrixTest : public ::testing::TestWithParam<int> {
public:
    SparseMatrixTest() : path("Test_sparse_matrix.h5"), nr(10), nc(5) {
        data = std::vector<double>{ -1.10, 0.18, 0.95, -0.17, -0.031, -0.75, 0.13, -0.89, 0.74, -0.43 };
        indices = std::vector<int>{ 0, 4, 4, 5, 7, 8, 4, 9, 2, 9 };
        indptr = std::vector<int>{ 0, 2, 5, 6, 8, 10 };
    }

protected:
    std::string path;
    int nr, nc;
    std::vector<double> data;
    std::vector<int> indices;
    std::vector<int> indptr;

    H5::Group sparse_matrix_opener(H5::Group& handle, int version) const {
        auto ghandle = array_opener(handle, "foobar", "sparse matrix");
        add_version_string(ghandle, version);

        auto dhandle = add_numeric_vector(ghandle, "data", data, H5::PredType::NATIVE_DOUBLE);
        if (version < 1100000) {
            add_numeric_vector<int>(ghandle, "shape", { nr, nc }, H5::PredType::NATIVE_INT);
            add_numeric_vector(ghandle, "indices", indices, H5::PredType::NATIVE_INT);
            add_numeric_vector(ghandle, "indptr", indptr, H5::PredType::NATIVE_INT);
        } else {
            add_numeric_vector<int>(ghandle, "shape", { nr, nc }, H5::PredType::NATIVE_UINT32);
            add_numeric_vector(ghandle, "indices", indices, H5::PredType::NATIVE_UINT32);
            add_numeric_vector(ghandle, "indptr", indptr, H5::PredType::NATIVE_UINT64);
            add_string_attribute(dhandle, "type", "FLOAT");
            add_numeric_scalar(ghandle, "by_column", 1, H5::PredType::NATIVE_INT8);
        }

        return ghandle;
    } 
};

TEST_P(SparseMatrixTest, Basic) {
    auto version = GetParam();

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        sparse_matrix_opener(fhandle, version);
    }
    {
        auto output = chihaya::validate(path, "foobar"); 
        EXPECT_EQ(output.type, chihaya::FLOAT);
        const auto& dims = output.dimensions;
        EXPECT_EQ(dims.size(), 2);
        EXPECT_EQ(dims[0], 10);
        EXPECT_EQ(dims[1], 5);
    }

    // Works for CSR if we flip the rows and columns.
    if (version >= 1100000) {
        {
            H5::H5File fhandle(path, H5F_ACC_TRUNC);
            auto ghandle = array_opener(fhandle, "foobar", "sparse matrix");
            add_version_string(ghandle, version);

            auto dhandle = add_numeric_vector(ghandle, "data", data, H5::PredType::NATIVE_DOUBLE);
            add_numeric_vector<int>(ghandle, "shape", { nc, nr }, H5::PredType::NATIVE_UINT32);
            add_numeric_vector(ghandle, "indices", indices, H5::PredType::NATIVE_UINT32);
            add_numeric_vector(ghandle, "indptr", indptr, H5::PredType::NATIVE_UINT64);
            add_string_attribute(dhandle, "type", "FLOAT");
            add_numeric_scalar(ghandle, "by_column", 0, H5::PredType::NATIVE_INT8);
        }
        auto output = chihaya::validate(path, "foobar"); 
        EXPECT_EQ(output.type, chihaya::FLOAT);
        const auto& dims = output.dimensions;
        EXPECT_EQ(dims.size(), 2);
        EXPECT_EQ(dims[0], 5);
        EXPECT_EQ(dims[1], 10);
    }
}

TEST_P(SparseMatrixTest, Dimnames) {
    auto version = GetParam();

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = sparse_matrix_opener(fhandle, version);
        auto lhandle = list_opener(ghandle, "dimnames", 2, version);
        add_string_vector(lhandle, "0", nr, /* len = */ 2);
        add_string_vector(lhandle, "1", nc, /* len = */ 2);
    }
    auto output = chihaya::validate(path, "foobar"); 
    EXPECT_EQ(output.type, chihaya::FLOAT);
}

TEST_P(SparseMatrixTest, Boolean) {
    auto version = GetParam();

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = sparse_matrix_opener(fhandle, version);
        ghandle.unlink("data");

        std::vector<int> copy(data.begin(), data.end());
        if (version < 1100000) {
            auto dhandle = add_numeric_vector<int>(ghandle, "data", copy, H5::PredType::NATIVE_INT); 
            auto ahandle = dhandle.createAttribute("is_boolean", H5::PredType::NATIVE_INT, H5S_SCALAR);
            int val = 1;
            ahandle.write(H5::PredType::NATIVE_INT, &val);
        } else {
            auto dhandle = add_numeric_vector<int>(ghandle, "data", copy, H5::PredType::NATIVE_INT8); 
            add_string_attribute(dhandle, "type", "BOOLEAN");
        }
    }

    auto output = chihaya::validate(path, "foobar"); 
    EXPECT_EQ(output.type, chihaya::BOOLEAN);
}

TEST_P(SparseMatrixTest, Missing) {
    auto version = GetParam();

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = sparse_matrix_opener(fhandle, version);
        auto dhandle = ghandle.openDataSet("data");
        if (version >= 1100000) {
            add_numeric_missing_placeholder(dhandle, 2.5, H5::PredType::NATIVE_DOUBLE);
        } else {
            add_numeric_missing_placeholder(dhandle, 2.5, H5::PredType::NATIVE_FLOAT);
        }
    }

    auto output = chihaya::validate(path, "foobar"); 
    EXPECT_EQ(output.type, chihaya::FLOAT);
}

TEST_P(SparseMatrixTest, ShapeErrors) {
    auto version = GetParam();

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = array_opener(fhandle, "foobar", "sparse matrix");
        add_version_string(ghandle, version);
    }
    expect_error([&]() -> void { chihaya::validate(path, "foobar"); }, "expected a dataset at 'shape'");

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = array_opener(fhandle, "foobar", "sparse matrix");
        add_version_string(ghandle, version);
        add_numeric_vector<int>(ghandle, "shape", { 10, 5, 2 }, H5::PredType::NATIVE_UINT8);
    }
    expect_error([&]() -> void { chihaya::validate(path, "foobar"); }, "'shape' should have length 2");

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = array_opener(fhandle, "foobar", "sparse matrix");
        add_version_string(ghandle, version);
        add_numeric_vector<int>(ghandle, "shape", { nr, nc }, H5::PredType::NATIVE_DOUBLE);
    }
    if (version < 1100000) {
        expect_error([&]() -> void { chihaya::validate(path, "foobar"); }, "'shape' should be integer");
    } else {
        expect_error([&]() -> void { chihaya::validate(path, "foobar"); }, "64-bit unsigned integer");
    }

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = array_opener(fhandle, "foobar", "sparse matrix");
        add_version_string(ghandle, version);
        add_numeric_vector<int>(ghandle, "shape", { -1, nc }, H5::PredType::NATIVE_INT);
    }
    if (version < 1100000) {
        expect_error([&]() -> void { chihaya::validate(path, "foobar"); }, "'shape' should contain non-negative");
    } else {
        expect_error([&]() -> void { chihaya::validate(path, "foobar"); }, "64-bit unsigned integer");
    }
}

TEST_P(SparseMatrixTest, DataErrors) {
    auto version = GetParam();

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = sparse_matrix_opener(fhandle, version);
        ghandle.unlink("data");
    }
    expect_error([&]() -> void { chihaya::validate(path, "foobar"); }, "expected a dataset at 'data'");

    {
        H5::H5File fhandle(path, H5F_ACC_RDWR);
        auto ghandle = fhandle.openGroup("foobar");
        ghandle.createDataSet("data", H5::PredType::NATIVE_INT, H5S_SCALAR);
    }
    expect_error([&]() -> void { chihaya::validate(path, "foobar"); }, "expected a 1-dimensional dataset");

    {
        H5::H5File fhandle(path, H5F_ACC_RDWR);
        auto ghandle = fhandle.openGroup("foobar");
        ghandle.unlink("data");

        auto dhandle = add_string_vector(ghandle, "data", 20);
        if (version >= 1100000) {
            add_string_attribute(dhandle, "type", "STRING");
        }
    }
    expect_error([&]() -> void { chihaya::validate(path, "foobar"); }, "integer, float or boolean");
}

TEST_P(SparseMatrixTest, SimpleIndexErrors) {
    auto version = GetParam();

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = sparse_matrix_opener(fhandle, version);
        ghandle.unlink("indices");
    }
    expect_error([&]() -> void { chihaya::validate(path, "foobar"); }, "expected a dataset at 'indices'");

    {
        H5::H5File fhandle(path, H5F_ACC_RDWR);
        auto ghandle = fhandle.openGroup("foobar");
        add_numeric_vector<double>(ghandle, "indices", { 1, 2 }, H5::PredType::NATIVE_DOUBLE);
    }
    if (version < 1100000) {
        expect_error([&]() -> void { chihaya::validate(path, "foobar"); }, "'indices' should be integer");
    } else {
        expect_error([&]() -> void { chihaya::validate(path, "foobar"); }, "64-bit unsigned integer");
    }

    {
        H5::H5File fhandle(path, H5F_ACC_RDWR);
        auto ghandle = fhandle.openGroup("foobar");
        ghandle.unlink("indices");
        add_numeric_vector<int>(ghandle, "indices", { 1, 2 }, H5::PredType::NATIVE_UINT32);
        ghandle.unlink("indptr");
    }
    expect_error([&]() -> void { chihaya::validate(path, "foobar"); }, "same length");
}

TEST_P(SparseMatrixTest, IndptrErrors) {
    auto version = GetParam();

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = sparse_matrix_opener(fhandle, version);
        ghandle.unlink("indptr");
    }
    expect_error([&]() -> void { chihaya::validate(path, "foobar"); }, "expected a dataset at 'indptr'");

    {
        H5::H5File fhandle(path, H5F_ACC_RDWR);
        auto ghandle = fhandle.openGroup("foobar");
        add_numeric_vector<double>(ghandle, "indptr", { 0 }, H5::PredType::NATIVE_DOUBLE);
    }
    if (version < 1100000) {
        expect_error([&]() -> void { chihaya::validate(path, "foobar"); }, "'indptr' should be integer");
    } else {
        expect_error([&]() -> void { chihaya::validate(path, "foobar"); }, "64-bit unsigned integer");
    }

    {
        H5::H5File fhandle(path, H5F_ACC_RDWR);
        auto ghandle = fhandle.openGroup("foobar");
        ghandle.unlink("indptr");
        add_numeric_vector<int>(ghandle, "indptr", { 0 }, H5::PredType::NATIVE_UINT32);
    }
    expect_error([&]() -> void { chihaya::validate(path, "foobar"); }, "'indptr' should have length");

    {
        H5::H5File fhandle(path, H5F_ACC_RDWR);
        auto ghandle = fhandle.openGroup("foobar");
        ghandle.unlink("indptr");
        auto copy = indptr;
        copy[0] = 1;
        add_numeric_vector<int>(ghandle, "indptr", copy, H5::PredType::NATIVE_UINT32);
    }
    expect_error([&]() -> void { chihaya::validate(path, "foobar"); }, "first entry");

    {
        H5::H5File fhandle(path, H5F_ACC_RDWR);
        auto ghandle = fhandle.openGroup("foobar");
        ghandle.unlink("indptr");
        auto copy = indptr;
        copy.back() = 1;
        add_numeric_vector<int>(ghandle, "indptr", copy, H5::PredType::NATIVE_UINT32);
    }
    expect_error([&]() -> void { chihaya::validate(path, "foobar"); }, "last entry");

    {
        H5::H5File fhandle(path, H5F_ACC_RDWR);
        auto ghandle = fhandle.openGroup("foobar");
        ghandle.unlink("indptr");
        auto copy = indptr;
        copy[2] = copy[1] - 1;
        add_numeric_vector<int>(ghandle, "indptr", copy, H5::PredType::NATIVE_UINT32);
    }
    expect_error([&]() -> void { chihaya::validate(path, "foobar"); }, "sorted");
}

TEST_P(SparseMatrixTest, ComplexIndexErrors) {
    auto version = GetParam();

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = sparse_matrix_opener(fhandle, version);
        ghandle.unlink("indices");
        auto copy = indices;
        copy[0] = nr + 10;
        add_numeric_vector<int>(ghandle, "indices", copy, H5::PredType::NATIVE_UINT16);
    }
    expect_error([&]() -> void { chihaya::validate(path, "foobar"); }, "number of rows");

    if (version < 1100000) {
        {
            H5::H5File fhandle(path, H5F_ACC_RDWR);
            auto ghandle = fhandle.openGroup("foobar");
            ghandle.unlink("indices");
            auto copy = indices;
            copy[0] = -1;
            add_numeric_vector<int>(ghandle, "indices", copy, H5::PredType::NATIVE_INT);
        }
        expect_error([&]() -> void { chihaya::validate(path, "foobar"); }, "non-negative");
    }

    {
        H5::H5File fhandle(path, H5F_ACC_RDWR);
        auto ghandle = fhandle.openGroup("foobar");
        ghandle.unlink("indices");
        auto copy = indices;
        std::fill(copy.begin(), copy.end(), 0);
        add_numeric_vector<int>(ghandle, "indices", copy, H5::PredType::NATIVE_UINT16);
    }
    expect_error([&]() -> void { chihaya::validate(path, "foobar"); }, "strictly increasing");
}

TEST_P(SparseMatrixTest, MissingErrors) {
    auto version = GetParam();

    if (version >= 1000000) {
        {
            H5::H5File fhandle(path, H5F_ACC_TRUNC);
            auto ghandle = sparse_matrix_opener(fhandle, version);
            auto dhandle = ghandle.openDataSet("data");
            add_numeric_missing_placeholder(dhandle, 1, H5::PredType::NATIVE_INT32);
        }
        if (version < 1100000) {
            expect_error([&]() -> void { chihaya::validate(path, "foobar"); }, "same type class");
        } else {
            expect_error([&]() -> void { chihaya::validate(path, "foobar"); }, "same type as");
        }
    }

    if (version >= 1100000) {
        {
            H5::H5File fhandle(path, H5F_ACC_TRUNC);
            auto ghandle = sparse_matrix_opener(fhandle, version);
            auto dhandle = ghandle.openDataSet("data");
            add_numeric_missing_placeholder(dhandle, 1, H5::PredType::NATIVE_FLOAT);
        }
        expect_error([&]() -> void { chihaya::validate(path, "foobar"); }, "same type as");
    }
}

INSTANTIATE_TEST_SUITE_P(
    SparseMatrix,
    SparseMatrixTest,
    ::testing::Values(0, 1000000, 1100000)
);
