#ifndef UTILS_H
#define UTILS_H

#include <gtest/gtest.h>

#include "H5Cpp.h"
#include "ritsuko/hdf5/hdf5.hpp"
#include "chihaya/chihaya.hpp"

#include <iostream>
#include <vector>
#include <string>
#include <type_traits>
#include <cstdint>

/*** HDF5-related utilities ***/

template<typename H5Obj_>
void add_string_attribute(const H5Obj_& handle, const std::string& name, const std::string& value, size_t len = H5T_VARIABLE) {
    H5::StrType stype(0, len);
    auto ahandle = handle.createAttribute(name, stype, H5S_SCALAR);
    ahandle.write(stype, value);
}

inline H5::Group operation_opener(const H5::Group& parent, const std::string& name, const std::string& operation) {
    auto ghandle = parent.createGroup(name);
    add_string_attribute(ghandle, "delayed_type", "operation");
    add_string_attribute(ghandle, "delayed_operation", operation);
    return ghandle;
}

inline H5::Group array_opener(const H5::Group& parent, const std::string& name, const std::string& array) {
    auto ghandle = parent.createGroup(name);
    add_string_attribute(ghandle, "delayed_type", "array");
    add_string_attribute(ghandle, "delayed_array", array);
    return ghandle;
}

template<typename T>
H5::DataSet add_numeric_vector(const H5::Group& handle, const std::string& name, const std::vector<T>& values, const H5::DataType& dtype) {
    hsize_t n = values.size();
    H5::DataSpace dspace(1, &n);
    auto dhandle = handle.createDataSet(name, dtype, dspace); 
    dhandle.write(values.data(), ritsuko::hdf5::as_numeric_datatype<T>());
    return dhandle;
}

inline H5::DataSet add_string_vector(const H5::Group& handle, const std::string& name, hsize_t n, hsize_t len = H5T_VARIABLE) {
    H5::DataSpace dspace(1, &n);
    return handle.createDataSet(name, H5::StrType(0, len), dspace); 
}

template<typename T>
H5::DataSet add_numeric_scalar(const H5::Group& handle, const std::string& name, T value, const H5::DataType& dtype) {
    auto dhandle = handle.createDataSet(name, dtype, H5S_SCALAR); 
    dhandle.write(&value, ritsuko::hdf5::as_numeric_datatype<T>());
    return dhandle;
}

inline H5::DataSet add_string_scalar(const H5::Group& handle, const std::string& name, const std::string& value, size_t len = H5T_VARIABLE) {
    H5::StrType stype(0, len);
    auto dhandle = handle.createDataSet(name, stype, H5S_SCALAR); 
    dhandle.write(value, stype);
    return dhandle;
}

inline H5::Group mock_array_opener(const H5::Group& parent, const std::string& name, const std::vector<int>& dimensions, int version, std::string type) {
    auto ghandle = array_opener(parent, name, "custom mock");
    add_string_scalar(ghandle, "type", type);

    if (version < 1100000) {
        add_numeric_vector(ghandle, "dimensions", dimensions, H5::PredType::NATIVE_INT);
    } else {
        add_numeric_vector(ghandle, "dimensions", dimensions, H5::PredType::NATIVE_UINT32);
    }

    return ghandle;
}

inline H5::Group list_opener(const H5::Group& parent, const std::string& name, int length, int version = 0) {
    auto ghandle = parent.createGroup(name);
    if (version < 1100000) {
        add_string_attribute(ghandle, "delayed_type", "list");
    }

    if (version < 1100000) {
        auto ahandle = ghandle.createAttribute("delayed_length", H5::PredType::NATIVE_INT, H5S_SCALAR);
        ahandle.write(H5::PredType::NATIVE_INT, &length);
    } else {
        auto ahandle = ghandle.createAttribute("length", H5::PredType::NATIVE_UINT32, H5S_SCALAR);
        ahandle.write(H5::PredType::NATIVE_INT, &length);
    }

    return ghandle;
}

template<typename T>
void add_numeric_missing_placeholder(const H5::DataSet& handle, T value, const H5::DataType& dtype) {
    auto dhandle = handle.createAttribute("missing_placeholder", dtype, H5S_SCALAR); 
    dhandle.write(ritsuko::hdf5::as_numeric_datatype<T>(), &value);
}

inline void add_string_missing_placeholder(const H5::DataSet& handle, const std::string& value, size_t len = H5T_VARIABLE) {
    add_string_attribute(handle, "missing_placeholder", value, len);
}

inline void add_version_string(const H5::Group& handle, int version) {
    if (version == 1000000) {
        add_string_attribute(handle, "delayed_version", "1.0.0");
    } else if (version >= 1100000) {
        add_string_attribute(handle, "delayed_version", "1.1.0");
    }
}

/*** Testing functions ***/

template<class Function>
void expect_error(Function op, std::string message) {
    EXPECT_ANY_THROW({ 
        try {
            op();
            std::cerr << "expected \"" << message << "\" for non-failing test" << std::endl;
        } catch (std::exception& e) {
            std::string msg(e.what());
            bool found = (msg.find(message) != std::string::npos);
            EXPECT_TRUE(found) << "expected \"" << message << "\" (got \"" << msg << "\")" << std::endl;
            throw;
        }
    });
}

chihaya::ArrayDetails test_validate(const std::string&, const std::string&);

inline void expect_error(const std::string& path, const std::string& name, std::string message) {
    expect_error([&]() { test_validate(path, name); }, std::move(message));
}

#endif
