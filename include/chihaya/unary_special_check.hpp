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
    ArrayDetails seed_details;
    try {
        auto shandle = group.openGroup("seed");
        seed_details = validate_numeric_seed(shandle, version, options);
    } catch (...) {
        std::throw_with_nested(std::runtime_error("failed to validate 'seed'"));
    }

    try {
        auto mhandle = group.openDataSet("method");
        auto method = read_scalar_string_dataset(mhandle);
        if (!options.details_only) {
            if (!is_valid_special_check_operation(method)) {
                throw std::runtime_error("unrecognized operation '" + method + "'");
            }
        }
    } catch (...) {
        std::throw_with_nested(std::runtime_error("failed to validate 'method'"));
    }

    seed_details.type = BOOLEAN;
    return seed_details;
}

}

#endif
