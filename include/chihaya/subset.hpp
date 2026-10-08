#ifndef CHIHAYA_SUBSET_HPP
#define CHIHAYA_SUBSET_HPP

#include "H5Cpp.h"
#include "ritsuko/ritsuko.hpp"

#include <exception>
#include <vector>
#include <stdexcept>

#include "utils_public.hpp"
#include "utils_misc.hpp"
#include "utils_subset.hpp"

/**
 * @file subset.hpp
 * @brief Validation for delayed subsets.
 */

namespace chihaya {

/**
 * @param group HDF5 group representing a subset operation.
 * @param version Verison of the **chihaya** specification.
 * @param options Validation options.
 *
 * @return Details of the subsetted object.
 *
 * @throw H5::Exception Thrown upon HDF5 library error.
 * This may be wrapped in a `std::nested_exception`.
 * @throw std::exception Thrown upon validation failure.
 * This may be a `std::nested_exception`, in which case the nested exceptions should be extracted to obtain an informative error message.
 */
inline ArrayDetails validate_subset(const H5::Group& group, const ritsuko::Version& version, const Options& options) {
    ArrayDetails seed_details;
    try {
        auto shandle = group.openGroup("seed");
        seed_details = validate(shandle, version, options);
    } catch (...) {
        std::throw_with_nested(std::runtime_error("failed to validate 'seed'"));
    }

    auto& seed_dims = seed_details.dimensions;
    std::vector<std::pair<std::size_t, std::size_t> > collected;
    try {
        auto ihandle = group.openGroup("index");
        collected = validate_subset_index_list(ihandle, seed_dims, version, options.contiguous_chunk_size);
    } catch (...) {
        std::throw_with_nested(std::runtime_error("failed to validate 'index'"));
    }

    for (auto p : collected) {
        seed_dims[p.first] = p.second;
    }
    return seed_details;
}

}

#endif
