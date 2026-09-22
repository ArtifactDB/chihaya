#ifndef CHIHAYA_UNARY_SPECIAL_CHECK_HPP
#define CHIHAYA_UNARY_SPECIAL_CHECK_HPP

#include "H5Cpp.h"
#include "ritsuko/ritsuko.hpp"

#include <stdexcept>

#include "utils_public.hpp"
#include "utils_misc.hpp"
#include "utils_nary.hpp"

/**
 * @file unary_special_check.hpp
 * @brief Validation for delayed unary special checks.
 */

namespace chihaya {

/**
 * @param group HDF5 group representing an unary special check operation.
 * @param version Version of the **chihaya** specification.
 * @param options Validation options.
 *
 * @return Details of the object after applying the special check.
 * Otherwise, if the validation failed, an error is raised.
 */
inline ArrayDetails validate_unary_special_check(const H5::Group& group, const ritsuko::Version& version, const Options& options) {
    auto seed_details = fetch_numeric_seed(group, "seed", version, options);

    // Checking the method.
    auto method = load_scalar_string_dataset(group, "method");
    if (!options.details_only) {
        if (!is_valid_special_check_operation(method)) {
            throw std::runtime_error("unrecognized 'method' (" + method + ")");
        }
    }

    seed_details.type = BOOLEAN;
    return seed_details;
}

}

#endif
