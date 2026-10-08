#ifndef CHIHAYA_BINARY_COMPARISON_HPP
#define CHIHAYA_BINARY_COMPARISON_HPP

#include "H5Cpp.h"
#include "ritsuko/ritsuko.hpp"

#include <stdexcept>
#include <exception>
#include <vector>

#include "utils_public.hpp"
#include "utils_misc.hpp"
#include "utils_nary.hpp"

/**
 * @file binary_comparison.hpp
 * @brief Validation for delayed binary comparisons.
 */

namespace chihaya {

/**
 * @param group HDF5 group representing a binary comparison.
 * @param version Version of the **chihaya** specification.
 * @param options Validation options.
 *
 * @return Details of the object after applying the comparison operation.
 * Otherwise, if the validation failed, an exception is thrown.
 * This exception may be nested.
 */
inline ArrayDetails validate_binary_comparison(const H5::Group& group, const ritsuko::Version& version, const Options& options) {
    ArrayDetails left_details;
    try {
        auto lhandle = group.openGroup("left");
        left_details = validate(lhandle, version, options);
    } catch (...) {
        std::throw_with_nested(std::runtime_error("failed to validate 'left'"));
    }

    ArrayDetails right_details;
    try {
        auto rhandle = group.openGroup("right");
        right_details = validate(rhandle, version, options);
    } catch (...) {
        std::throw_with_nested(std::runtime_error("failed to validate 'right'"));
    }

    if (!options.details_only) {
        if (!are_dimensions_equal(left_details.dimensions, right_details.dimensions)) {
            throw std::runtime_error("'left' and 'right' should have the same dimensions");
        }

        if ((left_details.type == STRING) != (right_details.type == STRING)) {
            throw std::runtime_error("both or neither of 'left' and 'right' should contain strings");
        }

        try {
            auto mhandle = group.openDataSet("method");
            auto method = read_scalar_string_dataset(mhandle);
            if (!is_valid_comparison_operation(method)) {
                throw std::runtime_error("unrecognized operation '" + method + "'");
            }
        } catch (...) {
            std::throw_with_nested(std::runtime_error("failed to validate 'method'"));
        }
    }

    left_details.type = BOOLEAN;
    return left_details;
}

}

#endif
