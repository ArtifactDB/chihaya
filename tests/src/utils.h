#ifndef UTILS_H
#define UTILS_H

#include <gtest/gtest.h>

#include "H5Cpp.h"
#include "ritsuko/ritsuko.hpp"
#include "chihaya/utils_public.hpp"

#include <iostream>
#include <vector>
#include <string>
#include <type_traits>
#include <cstdint>
#include <filesystem>

/*** HDF5-related utilities ***/

template<typename H5Object_>
void add_string_attribute(const H5Object_& handle, const std::string& name, const std::string& value, size_t len = H5T_VARIABLE) {
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

template<typename Value_>
H5::DataSet add_numeric_vector(const H5::Group& handle, const std::string& name, const std::vector<Value_>& values, const H5::DataType& dtype) {
    hsize_t n = values.size();
    H5::DataSpace dspace(1, &n);
    auto dhandle = handle.createDataSet(name, dtype, dspace); 
    dhandle.write(values.data(), ritsuko::hdf5::as_numeric_datatype<Value_>());
    return dhandle;
}

inline H5::DataSet add_string_vector(const H5::Group& handle, const std::string& name, hsize_t n, hsize_t len = H5T_VARIABLE) {
    H5::DataSpace dspace(1, &n);
    return handle.createDataSet(name, H5::StrType(0, len), dspace); 
}

template<typename Value_>
H5::DataSet add_numeric_scalar(const H5::Group& handle, const std::string& name, Value_ value, const H5::DataType& dtype) {
    auto dhandle = handle.createDataSet(name, dtype, H5S_SCALAR); 
    dhandle.write(&value, ritsuko::hdf5::as_numeric_datatype<Value_>());
    return dhandle;
}

inline H5::DataSet add_string_scalar(const H5::Group& handle, const std::string& name, const std::string& value, size_t len = H5T_VARIABLE) {
    H5::StrType stype(0, len);
    auto dhandle = handle.createDataSet(name, stype, H5S_SCALAR); 
    dhandle.write(value, stype);
    return dhandle;
}

template<typename Dim_>
H5::Group mock_array_opener(const H5::Group& parent, const std::string& name, const std::vector<Dim_>& dimensions, const ritsuko::Version& version, const std::string& type) {
    auto ghandle = array_opener(parent, name, "custom mock");
    add_string_scalar(ghandle, "type", type);
    if (version.lt(1, 1, 0)) {
        add_numeric_vector(ghandle, "dimensions", dimensions, H5::PredType::NATIVE_INT);
    } else {
        add_numeric_vector(ghandle, "dimensions", dimensions, H5::PredType::NATIVE_UINT32);
    }
    return ghandle;
}

inline H5::Group list_opener(const H5::Group& parent, const std::string& name, const int length, const ritsuko::Version& version) {
    auto ghandle = parent.createGroup(name);
    if (version.lt(1, 1, 0)) {
        add_string_attribute(ghandle, "delayed_type", "list");
        auto ahandle = ghandle.createAttribute("delayed_length", H5::PredType::NATIVE_INT, H5S_SCALAR);
        ahandle.write(H5::PredType::NATIVE_INT, &length);
    } else {
        auto ahandle = ghandle.createAttribute("length", H5::PredType::NATIVE_UINT32, H5S_SCALAR);
        ahandle.write(H5::PredType::NATIVE_INT, &length);
    }
    return ghandle;
}

template<typename Value_>
void add_numeric_missing_placeholder(const H5::DataSet& handle, Value_ value, const H5::DataType& dtype) {
    auto dhandle = handle.createAttribute("missing_placeholder", dtype, H5S_SCALAR); 
    dhandle.write(ritsuko::hdf5::as_numeric_datatype<Value_>(), &value);
}

inline void add_string_missing_placeholder(const H5::DataSet& handle, const std::string& value, size_t len = H5T_VARIABLE) {
    add_string_attribute(handle, "missing_placeholder", value, len);
}

inline void add_version_string(const H5::Group& handle, const ritsuko::Version& version) {
    if (version.eq(1, 0, 0)) {
        // Adding a patch number to test back-compatibility.
        add_string_attribute(handle, "delayed_version", "1.0.0");
    } else if (version.gt(1, 0, 0)) {
        add_string_attribute(handle, "delayed_version", std::to_string(version.major) + "." + std::to_string(version.minor));
    } else {
        // Version 0.99 didn't even have a version concept.
    }
}

/*** Testing functions ***/

inline auto spawn_all_versions() {
    return ::testing::Values(ritsuko::Version(0, 99, 0), ritsuko::Version(1, 0, 0), ritsuko::Version(1, 1, 0));
}

template<class Function_>
void expect_error(Function_ op, std::string message) {
    std::string msg;
    try {
        op();
        std::cerr << "expected \"" << message << "\" for non-failing test" << std::endl;
    } catch (std::exception& e) {
        msg = e.what();
    }
    bool found = (msg.find(message) != std::string::npos);
    EXPECT_TRUE(found) << "expected \"" << message << "\" (got \"" << msg << "\")" << std::endl;
}

chihaya::ArrayDetails test_validate(const std::string&, const std::string&, bool);

inline void expect_error(const std::string& path, const std::string& name, std::string message) {
    expect_error([&]() { test_validate(path, name, false); }, std::move(message));
}

inline std::string define_test_path(const std::string& suffix) {
    const std::string dir = "h5-test-files";
    std::filesystem::create_directory(dir);
    return dir + std::filesystem::path::preferred_separator + suffix + ".h5";
}

#endif
