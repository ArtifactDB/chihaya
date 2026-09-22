#ifndef CHIHAYA_DIMNAMES_HPP
#define CHIHAYA_DIMNAMES_HPP

#include "H5Cpp.h"
#include "ritsuko/ritsuko.hpp"

#include <stdexcept>

#include "utils_public.hpp"
#include "utils_dimensions.hpp"

/**
 * @file dimnames.hpp
 * @brief Validation for delayed dimnames assignment.
 */

namespace chihaya {

/**
 * @param group HDF5 group representing a dimnames assignment operation.
 * @param version Version of the **chihaya** specification.
 * @param options Validation options.
 *
 * @return Details of the object after assigning dimnames.
 * Otherwise, if the validation failed, an error is raised.
 */
inline ArrayDetails validate_dimnames(const H5::Group& group, const ritsuko::Version& version, Options& options) {
    ArrayDetails seed_details = fetch_seed(group, "seed", version, options);
    if (!options.details_only) {
        validate_dimnames_internal(group, seed_details.dimensions, version);
    }
    return seed_details;
}

}

#endif
