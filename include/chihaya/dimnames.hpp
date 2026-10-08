#ifndef CHIHAYA_DIMNAMES_HPP
#define CHIHAYA_DIMNAMES_HPP

#include "H5Cpp.h"
#include "ritsuko/ritsuko.hpp"

#include <stdexcept>
#include <exception>

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
 * Otherwise, if the validation failed, an exception is thrown.
 * This exception may be nested.
 */
inline ArrayDetails validate_dimnames(const H5::Group& group, const ritsuko::Version& version, const Options& options) {
    ArrayDetails seed_details;
    try {
        auto shandle = group.openGroup("seed");
        seed_details = validate(shandle, version, options);
    } catch (...) {
        std::throw_with_nested(std::runtime_error("failed to validate 'seed'")); 
    }

    if (!options.details_only) {
        try {
            auto nhandle = group.openGroup("dimnames");
            validate_dimnames_internal(nhandle, seed_details.dimensions, version, options.contiguous_chunk_size);
        } catch (...) {
            std::throw_with_nested(std::runtime_error("failed to validate 'dimnames'")); 
        }
    }

    return seed_details;
}

}

#endif
