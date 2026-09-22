#ifndef CHIHAYA_BINARY_LOGIC_HPP
#define CHIHAYA_BINARY_LOGIC_HPP

#include "H5Cpp.h"
#include "ritsuko/ritsuko.hpp"

#include <stdexcept>
#include <vector>
#include <string>

#include "utils_public.hpp"
#include "utils_misc.hpp"
#include "utils_nary.hpp"

/**
 * @file binary_logic.hpp
 * @brief Validation for delayed binary logical operations.
 */

namespace chihaya {

/**
 * @param group HDF5 group representing a binary logical operation.
 * @param version Version of the **chihaya** specification.
 * @param options Validation options.
 *
 * @return Details of the object after applying the logical operation.
 * Otherwise, if the validation failed, an error is raised.
 */
inline ArrayDetails validate_binary_logic(const H5::Group& group, const ritsuko::Version& version, Options& options) {
    auto left_details = fetch_numeric_seed(group, "left", version, options);
    auto right_details = fetch_numeric_seed(group, "right", version, options);

    if (!options.details_only) {
        if (!are_dimensions_equal(left_details.dimensions, right_details.dimensions)) {
            throw std::runtime_error("'left' and 'right' should have the same dimensions");
        }

        const auto method = load_scalar_string_dataset(group, "method");
        if (!is_valid_logic_operation(method)) {
            throw std::runtime_error("unrecognized 'method' (" + method + ")");
        }
    }

    left_details.type = BOOLEAN;
    return left_details;
}

}

#endif
