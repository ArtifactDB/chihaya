#include <gtest/gtest.h>
#include "chihaya/chihaya.hpp"
#include "utils.h"

template<class T>
H5::Group dense_array_opener(H5::Group& handle, std::string name, std::vector<hsize_t> dimensions, const T& type) {
    auto ghandle = array_opener(handle, name, "dense array");
    H5::DataSpace dspace(dimensions.size(), dimensions.data());
    ghandle.createDataSet("data", type, dspace);
    return ghandle;
}

std::vector<double> data { -1.10, 0.18, 0.95, -0.17, -0.031, -0.75, 0.13, -0.89, 0.74, -0.43 };
std::vector<int> indices { 0, 4, 4, 5, 7, 8, 4, 9, 2, 9 };
std::vector<int> indptr { 0, 2, 5, 6, 8, 10 };

TEST(Sparse, Basic) {
    std::string path = "Test_sparse_matrix.h5";

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = array_opener(fhandle, "foobar", "sparse matrix");
        add_vector<int>(ghandle, "shape", { 10, 5 });
        add_vector<double>(ghandle, "data", data);
        add_vector<int>(ghandle, "indices", indices);
        add_vector<int>(ghandle, "indptr", indptr);
    }
    {
        auto output = chihaya::validate(path, "foobar"); 
        EXPECT_EQ(output.type, chihaya::FLOAT);
        const auto& dims = output.dimensions;
        EXPECT_EQ(dims.size(), 2);
        EXPECT_EQ(dims[0], 10);
        EXPECT_EQ(dims[1], 5);
    }
}

TEST(Sparse, Dimnames) {
    std::string path = "Test_sparse_matrix.h5";

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = array_opener(fhandle, "foobar", "sparse matrix");
        add_vector<int>(ghandle, "shape", { 10, 5 });
        add_vector<double>(ghandle, "data", data);
        add_vector<int>(ghandle, "indices", indices);
        add_vector<int>(ghandle, "indptr", indptr);

        auto lhandle = list_opener(ghandle, "dimnames", 2);
        add_string_vector(lhandle, "0", 10);
        add_string_vector(lhandle, "1", 5);
    }
    auto output = chihaya::validate(path, "foobar"); 
    EXPECT_EQ(output.type, chihaya::FLOAT);
}

TEST(Sparse, Boolean) {
    std::string path = "Test_dense_array.h5";

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = array_opener(fhandle, "foobar", "sparse matrix");
        add_vector<int>(ghandle, "shape", { 10, 5 });

        std::vector<int> copy(data.begin(), data.end());
        add_vector<int>(ghandle, "data", copy); 

        add_vector<int>(ghandle, "indices", indices);
        add_vector<int>(ghandle, "indptr", indptr);

        auto dhandle = ghandle.openDataSet("data");
        auto ahandle = dhandle.createAttribute("is_boolean", H5::PredType::NATIVE_INT, H5S_SCALAR);
        int val = 1;
        ahandle.write(H5::PredType::NATIVE_INT, &val);
    }
    {
        auto output = chihaya::validate(path, "foobar"); 
        EXPECT_EQ(output.type, chihaya::BOOLEAN);
    }
}

TEST(Sparse, Missing) {
    std::string path = "Test_sparse_matrix.h5";

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = array_opener(fhandle, "foobar", "sparse matrix");
        add_version_string(fhandle.openGroup("foobar"), "1.0.0");

        add_vector<int>(ghandle, "shape", { 10, 5 });
        add_vector<double>(ghandle, "data", data);
        add_vector<int>(ghandle, "indices", indices);
        add_vector<int>(ghandle, "indptr", indptr);

        auto dhandle = ghandle.openDataSet("data");
        add_missing_placeholder(dhandle, 2.5);
    }
    {
        auto output = chihaya::validate(path, "foobar"); 
        EXPECT_EQ(output.type, chihaya::FLOAT);
    }
}


TEST(Sparse, ShapeErrors) {
    std::string path = "Test_sparse_matrix.h5";

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = array_opener(fhandle, "foobar", "sparse matrix");
    }
    expect_error([&]() -> void { chihaya::validate(path, "foobar"); }, "expected 'shape'");

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = array_opener(fhandle, "foobar", "sparse matrix");
        add_vector<double>(ghandle, "shape", { 10, 5 });
    }
    expect_error([&]() -> void { chihaya::validate(path, "foobar"); }, "'shape' should be integer");

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = array_opener(fhandle, "foobar", "sparse matrix");
        add_vector<int>(ghandle, "shape", { 10, 5, 2});
    }
    expect_error([&]() -> void { chihaya::validate(path, "foobar"); }, "'shape' should have length 2");

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = array_opener(fhandle, "foobar", "sparse matrix");
        add_vector<int>(ghandle, "shape", { -1, 2});
    }
    expect_error([&]() -> void { chihaya::validate(path, "foobar"); }, "'shape' should contain non-negative");
}

TEST(Sparse, DataErrors) {
    std::string path = "Test_sparse_matrix.h5";

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = array_opener(fhandle, "foobar", "sparse matrix");
        add_vector<int>(ghandle, "shape", { 10, 5 });
    }
    expect_error([&]() -> void { chihaya::validate(path, "foobar"); }, "expected 'data'");

    {
        H5::H5File fhandle(path, H5F_ACC_RDWR);
        auto ghandle = fhandle.openGroup("foobar");
        ghandle.createGroup("data");
    }
    expect_error([&]() -> void { chihaya::validate(path, "foobar"); }, "'data' to be a dataset");

    {
        H5::H5File fhandle(path, H5F_ACC_RDWR);
        auto ghandle = fhandle.openGroup("foobar");
        ghandle.unlink("data");
        ghandle.createDataSet("data", H5::PredType::NATIVE_INT, H5S_SCALAR);
    }
    expect_error([&]() -> void { chihaya::validate(path, "foobar"); }, "'data' should be a 1-dimensional dataset");
}

TEST(Sparse, SimpleIndexErrors) {
    std::string path = "Test_sparse_matrix.h5";

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = array_opener(fhandle, "foobar", "sparse matrix");
        add_vector<int>(ghandle, "shape", { 10, 5 });
        add_vector<double>(ghandle, "data", data);
    }
    expect_error([&]() -> void { chihaya::validate(path, "foobar"); }, "expected 'indices'");

    {
        H5::H5File fhandle(path, H5F_ACC_RDWR);
        auto ghandle = fhandle.openGroup("foobar");
        add_vector<double>(ghandle, "indices", { 1, 2 });
    }
    expect_error([&]() -> void { chihaya::validate(path, "foobar"); }, "'indices' should be integer");

    {
        H5::H5File fhandle(path, H5F_ACC_RDWR);
        auto ghandle = fhandle.openGroup("foobar");
        ghandle.unlink("indices");
        add_vector<int>(ghandle, "indices", { 1, 2 });
    }
    expect_error([&]() -> void { chihaya::validate(path, "foobar"); }, "same length");

    {
        H5::H5File fhandle(path, H5F_ACC_RDWR);
        auto ghandle = fhandle.openGroup("foobar");
        ghandle.unlink("indices");
        add_vector<int>(ghandle, "indices", indices);
    }
    expect_error([&]() -> void { chihaya::validate(path, "foobar"); }, "expected 'indptr'");

    {
        H5::H5File fhandle(path, H5F_ACC_RDWR);
        auto ghandle = fhandle.openGroup("foobar");
        add_vector<double>(ghandle, "indptr", { 0 });
    }
    expect_error([&]() -> void { chihaya::validate(path, "foobar"); }, "'indptr' should be integer");

    {
        H5::H5File fhandle(path, H5F_ACC_RDWR);
        auto ghandle = fhandle.openGroup("foobar");
        ghandle.unlink("indptr");
        add_vector<int>(ghandle, "indptr", { 0 });
    }
    expect_error([&]() -> void { chihaya::validate(path, "foobar"); }, "'indptr' should have length");
}

TEST(Sparse, ComplexIndexErrors) {
    std::string path = "Test_sparse_matrix.h5";

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = array_opener(fhandle, "foobar", "sparse matrix");
        add_vector<int>(ghandle, "shape", { 10, 5 });
        add_vector<double>(ghandle, "data", data);
        add_vector<int>(ghandle, "indices", indices);
        auto copy = indptr;
        copy[0] = 1;
        add_vector<int>(ghandle, "indptr", copy);
    }
    expect_error([&]() -> void { chihaya::validate(path, "foobar"); }, "first entry");

    {
        H5::H5File fhandle(path, H5F_ACC_RDWR);
        auto ghandle = fhandle.openGroup("foobar");
        ghandle.unlink("indptr");
        auto copy = indptr;
        copy.back() = 1;
        add_vector<int>(ghandle, "indptr", copy);
    }
    expect_error([&]() -> void { chihaya::validate(path, "foobar"); }, "last entry");

    {
        H5::H5File fhandle(path, H5F_ACC_RDWR);
        auto ghandle = fhandle.openGroup("foobar");
        ghandle.unlink("indptr");
        auto copy = indptr;
        copy[2] = copy[1] - 1;
        add_vector<int>(ghandle, "indptr", copy);
    }
    expect_error([&]() -> void { chihaya::validate(path, "foobar"); }, "sorted");

    {
        H5::H5File fhandle(path, H5F_ACC_RDWR);
        auto ghandle = fhandle.openGroup("foobar");

        ghandle.unlink("indptr");
        add_vector<int>(ghandle, "indptr", indptr);

        ghandle.unlink("indices");
        auto copy = indices;
        copy[0] = -1;
        add_vector<int>(ghandle, "indices", copy);
    }
    expect_error([&]() -> void { chihaya::validate(path, "foobar"); }, "non-negative");

    {
        H5::H5File fhandle(path, H5F_ACC_RDWR);
        auto ghandle = fhandle.openGroup("foobar");
        ghandle.unlink("indices");
        auto copy = indices;
        copy[0] = 10;
        add_vector<int>(ghandle, "indices", copy);
    }
    expect_error([&]() -> void { chihaya::validate(path, "foobar"); }, "number of rows");

    {
        H5::H5File fhandle(path, H5F_ACC_RDWR);
        auto ghandle = fhandle.openGroup("foobar");
        ghandle.unlink("indices");
        auto copy = indices;
        std::fill(copy.begin(), copy.end(), 0);
        add_vector<int>(ghandle, "indices", copy);
    }
    expect_error([&]() -> void { chihaya::validate(path, "foobar"); }, "strictly increasing");

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = array_opener(fhandle, "foobar", "sparse matrix");
        add_version_string(fhandle.openGroup("foobar"), "1.0.0");

        add_vector<int>(ghandle, "shape", { 10, 5 });
        add_vector<double>(ghandle, "data", data);
        add_vector<int>(ghandle, "indices", indices);
        add_vector<int>(ghandle, "indptr", indptr);

        auto dhandle = ghandle.openDataSet("data");
        add_missing_placeholder(dhandle, 1);
    }
    expect_error([&]() -> void { chihaya::validate(path, "foobar"); }, "should be of the same type");
}
