#ifndef CHIHAYA_UTILS_NARY_HPP
#define CHIHAYA_UTILS_NARY_HPP

#include "H5Cpp.h"
#include "ritsuko/ritsuko.hpp"

#include <stdexcept>
#include <string>

#include "utils_public.hpp"
#include "utils_misc.hpp"
#include "utils_dimensions.hpp"

namespace chihaya {


namespace chihaya {

inline ArrayDetails fetch_numeric_seed(const H5::Group& handle, const std::string& target, const ritsuko::Version& version, Options& options) {
    auto output = fetch_seed(handle, target, version, options);
    if (output.type == STRING) {
        throw std::runtime_error("type of '" + target + "' should be integer, float or boolean");
    }
    return output;
}

inline bool is_valid_arithmetic_operation(const std::string& method) {
    return (
        method == "+" ||
        method == "-" ||
        method == "/" ||
        method == "*" || 
        method == "%/%" ||
        method == "^" ||
        method == "%%"
    );
}

inline ArrayType determine_arithmetic_output_type(const ArrayType& first, const ArrayType& second, const std::string& method) {
    if (method == "/") {
        return FLOAT;
    } else if (method == "%/%") {
        return INTEGER;
    }

    auto output = std::max(first, second);
    if (output == BOOLEAN) {
        return INTEGER;
    }

    return output;
}

inline bool is_valid_comparison_operation(const std::string& method) {
    return (
        method == "==" ||
        method == ">" ||
        method == "<" ||
        method == ">=" ||
        method == "<=" ||
        method == "!="
    );
}

inline bool is_valid_logic_operation(const std::string& method) {
    return method == "&&" || method == "||";
}

inline void check_unary_along(const H5::Group& handle, const ritsuko::Version& version, const std::vector<std::size_t>& seed_dimensions, hsize_t extent) {
    const auto along = load_along(handle, version);

    if (sanisizer::is_greater_than_or_equal(along, seed_dimensions.size())) {
        throw std::runtime_error("'along' should be less than the seed dimensionality");
    }

    if (!sanisizer::is_equal(extent, seed_dimensions[along])) {
        throw std::runtime_error("length should be equal to the dimension specified in 'along'");
    }
}

}

#endif
