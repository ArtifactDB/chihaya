#ifndef CHIHAYA_UTILS_TYPE_HPP
#define CHIHAYA_UTILS_TYPE_HPP

#include "H5Cpp.h"
#include "ritsuko/ritsuko.hpp"

#include <string>
#include <stdexcept>
#include <limits>
#include <cstdint>
#include <cstddef>

#include "utils_public.hpp"

namespace chihaya {

auto create_integer_error_0_99(const std::string& name, H5T_class_t type_class) {
    if (type_class == H5T_INTEGER) {
        return std::runtime_error("integer type for '" + name + "' is too large to read");
    } else {
        return std::runtime_error("expected an integer type for '" + name + "'");
    }
}

// Older versions didn't actually specify the integer type, so we just check we can load it into an 'int' of any type.

template<typename Output_>
Output_ load_non_negative_integer_scalar_0_99(const H5::DataSet& handle) {
    static_assert(std::is_integral<Output_>::value);

    if (!ritsuko::hdf5::exceeds_integer_limit(handle, 64, true)) {
        std::int64_t val;
        handle.read(&val, H5::PredType::NATIVE_INT64);
        if (val < 0) {
            throw std::runtime_error("expected a non-negative integer in '" + ritsuko::hdf5::get_name(handle) + "'");
        }
        return sanisizer::cast<Output_>(val);

    } else if (!ritsuko::hdf5::exceeds_integer_limit(handle, 64, false)) {
        std::uint64_t val;
        handle.read(&val, H5::PredType::NATIVE_UINT64);
        return sanisizer::cast<Output_>(val);

    } else {
        throw create_integer_error_0_99(ritsuko::hdf5::get_name(handle), handle.getTypeClass());
        return 0;
    }
}

template<typename Output_>
Output_ load_non_negative_integer_scalar_0_99(const H5::Attribute& handle) {
    static_assert(std::is_integral<Output_>::value);

    if (!ritsuko::hdf5::exceeds_integer_limit(handle, 64, true)) {
        std::int64_t val;
        handle.read(H5::PredType::NATIVE_INT64, &val);
        if (val < 0) {
            throw std::runtime_error("expected a non-negative integer in '" + ritsuko::hdf5::get_name(handle) + "'");
        }
        return sanisizer::cast<Output_>(val);

    } else if (!ritsuko::hdf5::exceeds_integer_limit(handle, 64, false)) {
        std::uint64_t val;
        handle.read(H5::PredType::NATIVE_UINT64, &val);
        return sanisizer::cast<Output_>(val);

    } else {
        throw create_integer_error_0_99(ritsuko::hdf5::get_name(handle), handle.getTypeClass());
        return 0;
    }
}

template<typename Output_>
std::vector<Output_> load_non_negative_integer_vector_0_99(const H5::DataSet& handle, hsize_t len) {
    std::vector<Output_> output;
    output.reserve(len);

    if (!ritsuko::hdf5::exceeds_integer_limit(handle, 64, true)) {
        auto vals = sanisizer::create<std::vector<std::int64_t> >(len);
        handle.read(vals.data(), H5::PredType::NATIVE_INT64);
        for (auto val : vals) {
            if (val < 0) {
                throw std::runtime_error("'" + ritsuko::hdf5::get_name(handle) + "' should contain non-negative values");
            }
            output.push_back(sanisizer::cast<std::size_t>(val));
        }

    } else if (!ritsuko::hdf5::exceeds_integer_limit(handle, 64, false)) {
        auto vals = sanisizer::create<std::vector<std::uint64_t> >(len);
        handle.read(vals.data(), H5::PredType::NATIVE_UINT64);
        for (auto val : vals) {
            output.push_back(sanisizer::cast<std::size_t>(val));
        }

    } else {
        throw create_integer_error_0_99(ritsuko::hdf5::get_name(handle), handle.getTypeClass());
    }

    return output;
}

// For booleans, the legacy value can be negative and is treated as truthy.

inline bool load_boolean_scalar_0_99(const H5::DataSet& handle) {
    if (!ritsuko::hdf5::exceeds_integer_limit(handle, 64, true)) {
        std::int64_t val;
        handle.read(&val, H5::PredType::NATIVE_INT64);
        return val != 0;
    } else if (!ritsuko::hdf5::exceeds_integer_limit(handle, 64, false)) {
        std::uint64_t val;
        handle.read(&val, H5::PredType::NATIVE_UINT64);
        return val != 0;
    } else {
        throw create_integer_error_0_99(ritsuko::hdf5::get_name(handle), handle.getTypeClass());
        return false;
    }
}

inline bool load_boolean_scalar_0_99(const H5::Attribute& handle) {
    if (!ritsuko::hdf5::exceeds_integer_limit(handle, 64, true)) {
        std::int64_t val;
        handle.read(H5::PredType::NATIVE_INT64, &val);
        return val != 0;
    } else if (!ritsuko::hdf5::exceeds_integer_limit(handle, 64, false)) {
        std::uint64_t val;
        handle.read(H5::PredType::NATIVE_UINT64, &val);
        return val != 0;
    } else {
        throw create_integer_error_0_99(ritsuko::hdf5::get_name(handle), handle.getTypeClass());
        return false;
    }
}

inline bool is_boolean_0_99(const H5::DataSet& handle) {
    if (!handle.attrExists("is_boolean")) {
        return false;
    } else {
        if (handle.getDataType().getClass() != H5T_INTEGER) {
            throw std::runtime_error("'is_boolean' attribute should only exist for integer datasets");
        }
        auto ahandle = handle.openAttribute("is_boolean");
        if (ahandle.getSpace().getSimpleExtentNdims() != 0) {
            throw std::runtime_error("'is_boolean' attribute should be a scalar");
        }
        return load_boolean_scalar_0_99(ahandle);
    }
}

inline ArrayType translate_type_0_99(H5T_class_t cls) {
    if (cls == H5T_FLOAT) {
        return FLOAT;
    } else if (cls == H5T_STRING) {
        return STRING;
    } else if (cls != H5T_INTEGER) {
        throw std::runtime_error("unsupported HDF5 datatype class");
    }
    return INTEGER;
}

inline ArrayType translate_type_1_1(const std::string& type) {
    if (type == "INTEGER") {
        return INTEGER;
    } else if (type == "BOOLEAN") {
        return BOOLEAN;
    } else if (type == "FLOAT") {
        return FLOAT;
    } else if (type != "STRING") {
        throw std::runtime_error("unknown type '" + type + "'");
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

}

#endif
