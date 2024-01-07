#ifndef CHIHAYA_UTILS_ARITHMETIC_HPP
#define CHIHAYA_UTILS_ARITHMETIC_HPP

#include "H5Cpp.h"
#include "ritsuko/hdf5/hdf5.hpp"
#include "ritsuko/ritsuko.hpp"
#include <stdexcept>
#include <string>
#include "utils_public.hpp"

namespace chihaya {

ArrayDetails validate(const H5::Group&, const ritsuko::Version&);

namespace internal_arithemetic {

inline ArrayDetails fetch_seed(const H5::Group& handle, const std::string& target, const ritsuko::Version& version) {
    const auto& ghandle = ritsuko::hdf5::open_group(handle, target.c_str());

    ArrayDetails output;
    try {
        output = ::chihaya::validate(ghandle, version);
    } catch (std::exception& e) {
        throw std::runtime_error("failed to validate '" + target + "'; " + std::string(e.what()));
    }

    if (output.type == STRING) {
        throw std::runtime_error("type of '" + target + "' should be integer, float or boolean");
    }
    return output;
}

inline bool is_valid_operation(const std::string& method) {
    return (method == "+" ||
        method == "-" ||
        method == "/" ||
        method == "*" || 
        method == "%/%" ||
        method == "^" ||
        method == "%%");
}

inline ArrayType determine_output_type(const ArrayType& first, const ArrayType& second, const std::string& method) {
    if (method == "/") {
        return FLOAT;
    } else if (method == "%/%") {
        return INTEGER;
    } else if (method == "%%") {
        if (first <= INTEGER && second <= INTEGER) {
            return INTEGER;
        } else {
            return FLOAT;
        }
    } else if (first == BOOLEAN && second == BOOLEAN) {
        return INTEGER;
    } else {
        return std::max(first, second);
    }
}

}

}

#endif
