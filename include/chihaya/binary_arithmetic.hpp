#ifndef CHIHAYA_BINARY_ARITHMETIC_HPP
#define CHIHAYA_BINARY_ARITHMETIC_HPP

#include "H5Cpp.h"
#include "ritsuko/ritsuko.hpp"

#include <stdexcept>
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
 * @param handle An open handle on a HDF5 group representing a binary arithmetic operation.
 * @param version Version of the **chihaya** specification.
 * @param options Validation options.
 *
 * @return Details of the object after applying the arithmetic operation.
 * Otherwise, if the validation failed, an error is raised.
 */
inline ArrayDetails validate_binary_arithmetic(const H5::Group& handle, const ritsuko::Version& version, Options& options) {
    auto left_details = fetch_numeric_seed(handle, "left", version, options);
    auto right_details = fetch_numeric_seed(handle, "right", version, options);

    if (!options.details_only) {
        if (!are_dimensions_equal(left_details.dimensions, right_details.dimensions)) {
            throw std::runtime_error("'left' and 'right' should have the same dimensions");
        }
    }

    auto method = load_scalar_string_dataset(handle, "method");
    if (!options.details_only) {
        if (!is_valid_arithmetic_operation(method)) {
            throw std::runtime_error("unrecognized 'method' (" + method + ")");
        }
    }

    left_details.type = determine_arithmetic_output_type(left_details.type, right_details.type, method);
    return left_details;
}

}

#endif
