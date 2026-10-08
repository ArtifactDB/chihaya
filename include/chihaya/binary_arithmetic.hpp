#ifndef CHIHAYA_BINARY_ARITHMETIC_HPP
#define CHIHAYA_BINARY_ARITHMETIC_HPP

#include "H5Cpp.h"
#include "ritsuko/ritsuko.hpp"

#include <stdexcept>
#include <exception>
#include <vector>
#include <string>

#include "utils_public.hpp"
#include "utils_misc.hpp"
#include "utils_nary.hpp"

/**
 * @file binary_arithmetic.hpp
 * @brief Validation for delayed binary arithmetic operations.
 */

namespace chihaya {

/**
 * @param group HDF5 group representing a binary arithmetic operation.
 * @param version Version of the **chihaya** specification.
 * @param options Validation options.
 *
 * @return Details of the object after applying the arithmetic operation.
 *
 * @throw H5::Exception Thrown upon HDF5 library error.
 * This may be wrapped in a `std::nested_exception`.
 * @throw std::exception Thrown upon validation failure.
 * This may be a `std::nested_exception`, in which case the nested exceptions should be extracted to obtain an informative error message.
 */
inline ArrayDetails validate_binary_arithmetic(const H5::Group& group, const ritsuko::Version& version, const Options& options) {
    ArrayDetails left_details;
    try {
        auto lhandle = group.openGroup("left");
        left_details = validate_numeric_seed(lhandle, version, options);
    } catch (...) {
        std::throw_with_nested(std::runtime_error("failed to validate 'left'"));
    }

    ArrayDetails right_details;
    try {
        auto rhandle = group.openGroup("right");
        right_details = validate_numeric_seed(rhandle, version, options);
    } catch (...) {
        std::throw_with_nested(std::runtime_error("failed to validate 'right'"));
    }

    if (!options.details_only) {
        if (!are_dimensions_equal(left_details.dimensions, right_details.dimensions)) {
            throw std::runtime_error("'left' and 'right' should have the same dimensions");
        }
    }

    std::string method;
    try {
        auto mhandle = group.openDataSet("method");
        method = read_scalar_string_dataset(mhandle);
        if (!options.details_only) {
            if (!is_valid_arithmetic_operation(method)) {
                throw std::runtime_error("unrecognized operation '" + method + "'");
            }
        }
    } catch (...) {
        std::throw_with_nested(std::runtime_error("failed to validate 'method'"));
    }

    left_details.type = determine_arithmetic_output_type(left_details.type, right_details.type, method);
    return left_details;
}

}

#endif
