#ifndef CHIHAYA_UTILS_TYPE_HPP
#define CHIHAYA_UTILS_TYPE_HPP

#include "H5Cpp.h"
#include "ritsuko/hdf5/hdf5.hpp"

#include <string>
#include <stdexcept>

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
        if (!ritsuko::hdf5::is_scalar(ahandle) || ahandle.getTypeClass() != H5T_INTEGER) {
            throw std::runtime_error("'is_boolean' attribute should be an integer scalar");
        }

        ahandle.read(H5::PredType::NATIVE_INT, &is_bool);
    }
    return is_bool;
}

inline std::string fetch_data_type(const H5::DataSet& handle) {
    auto thandle = ritsuko::hdf5::open_attribute(handle, "type");
    if (!ritsuko::hdf5::is_scalar(thandle)) {
        throw std::runtime_error("'type' should be a scalar");
    }
    if (!ritsuko::hdf5::is_utf8_string(thandle)) {
        throw std::runtime_error("'type' should have a datatype that can be represented by a UTF-8 encoded string");
    }
    return ritsuko::hdf5::load_scalar_string_attribute(thandle);
}

inline ArrayType translate_type_1_1(const std::string& type) {
    if (type == "INTEGER") {
        return INTEGER;
    } else if (type == "BOOLEAN") {
        return BOOLEAN;
    } else if (type == "FLOAT") {
        return FLOAT;
    }
    return STRING;
}

inline void check_type_1_1(const H5::DataSet& handle, ArrayType type) {
    if (type == INTEGER) {
        if (ritsuko::hdf5::exceeds_integer_limit(handle, 32, true)) {
            throw std::runtime_error("integer dataset should have a datatype that fits into a 32-bit signed integer");
        }
    } else if (type == BOOLEAN) {
        if (ritsuko::hdf5::exceeds_integer_limit(handle, 8, true)) {
            throw std::runtime_error("boolean dataset should have a datatype that fits into a 8-bit signed integer");
        }
    } else if (type == FLOAT) {
        if (ritsuko::hdf5::exceeds_float_limit(handle, 64)) {
            throw std::runtime_error("float dataset should have a datatype that fits into a 64-bit float");
        }
    } else if (type == STRING) {
        if (!ritsuko::hdf5::is_utf8_string(handle)) {
            throw std::runtime_error("string dataset should have a datatype that can be represented by a UTF-8 encoded string");
        }
    } else {
        throw std::runtime_error("as-yet-unsupported type");
    }
}

inline ArrayType translate_type_0_0(H5T_class_t cls) {
    if (cls == H5T_FLOAT) {
        return FLOAT;
    } else if (cls == H5T_STRING) {
        return STRING;
    } else if (cls != H5T_INTEGER) {
        throw std::runtime_error("unrecognized HDF5 datatype class");
    }
    return INTEGER;
}

}

}

#endif
