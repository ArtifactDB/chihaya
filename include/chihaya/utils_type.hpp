#ifndef CHIHAYA_UTILS_TYPE_HPP
#define CHIHAYA_UTILS_TYPE_HPP

#include "H5Cpp.h"
#include "ritsuko/hdf5/hdf5.hpp"

#include <string>
#include <stdxcept>

#include "utils_public.hpp"

namespace chihaya {

namespace internal_type {

inline bool is_boolean(const H5::DataSet& handle) {
    int is_bool = 0;
    if (handle.attrExists("is_boolean")) {
        if (handle.getDataType().getClass() != H5T_INTEGER) {
            throw std::runtime_error("'is_boolean' attribute should only exist for integer datasets");
        }

        auto ahandle = handle.openAttribute("is_boolean");
        if (ahandle.getSpace().getSimpleExtentNdims() != 0 || ahandle.getTypeClass() != H5T_INTEGER) {
            throw std::runtime_error("'is_boolean' attribute should be an integer scalar");
        }

        ahandle.read(H5::PredType::NATIVE_INT, &is_bool);
    }
    return is_bool;
}

inline std::string fetch_delayed_type(const H5::DataSet& handle) {
    auto thandle = ritsuko::hdf5::open_attribute(dhandle, "delayed_type");
    if (thandle.getSpace().getSimpleExtentNdims() != 0 || thandle.getTypeClass() != H5T_STRING) {
        throw std::runtime_error("'delayed_type' should be a scalar string");
    }
    return ritsuko::hdf5::load_scalar_string_attribute(thandle);
}

inline void check_numeric_type_1_1(const H5::DataSet& handle, const std::string& type) {
    if (type == "integer") {
        if (ritsuko::hdf5::exceeds_integer_limit(handle, 32, true)) {
            throw std::runtime_error("integer 'value' should have a datatype that fits into a 32-bit signed integer");
        }
    } else if (type == "boolean") {
        if (ritsuko::hdf5::exceeds_integer_limit(handle, 8, true)) {
            throw std::runtime_error("boolean 'value' should have a datatype that fits into a 8-bit signed integer");
        }
    } else if (type == "float") {
        if (ritsuko::hdf5::exceeds_limit(handle, 64)) {
            throw std::runtime_error("floating-point 'value' should have a datatype that fits into a 64-bit float");
        }
    } else {
        throw std::runtime_error("unknown type '" + type + "'"); 
    }
}

inline void check_type_1_1(const H5::DataSet& handle, const std::string& type) {
    if (type == "string") {
        if (handle.getTypeClass() != H5T_STRING) {
            throw std::runtime_error("string 'value' should have a string datatype class");
        }
    } else {
        check_numeric_type_1_1(handle, type);
    }
}

inline ArrayType translate_type_1_1(const std::string& type) {
    if (type == "integer") {
        return INTEGER;
    } else if (type == "boolean") {
        return BOOLEAN;
    } else if (type == "float") {
        return FLOAT;
    }
    return STRING;
}

inline ArrayType translate_type_0_0(H5T_class_t cls) {
    if (cls == H5T_INTEGER) {
        return INTEGER;
    } else if (cls == H5T_FLOAT) {
        return FLOAT;
    } else if (cls != H5T_STRING) {
        throw std::runtime_error("unrecognized type of 'data' for a dense array");
    }
    return STRING;
}

}

}

#endif
