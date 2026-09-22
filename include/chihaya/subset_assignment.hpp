#ifndef CHIHAYA_SUBSET_ASSIGNMENT_HPP
#define CHIHAYA_SUBSET_ASSIGNMENT_HPP

#include "H5Cpp.h"
#include "ritsuko/ritsuko.hpp"

#include <stdexcept>
#include <vector>
#include <algorithm>

#include "utils_public.hpp"
#include "utils_list.hpp"
#include "utils_misc.hpp"
#include "utils_subset.hpp"
#include "utils_dimensions.hpp"

/**
 * @file subset_assignment.hpp
 * @brief Validation for delayed subset assignment.
 */

namespace chihaya {

/**
 * @param group HDF5 group representing a subset assignment.
 * @param version Version of the **chihaya** specification.
 * @param options Validation options.
 *
 * @return Details of the object after subset assignment.
 * Otherwise, if the validation failed, an error is raised.
 */
inline ArrayDetails validate_subset_assignment(const H5::Group& group, const ritsuko::Version& version, const Options& options) {
    auto seed_details = fetch_seed(group, "seed", version, options);
    const auto& seed_dims = seed_details.dimensions;

    auto value_details = fetch_seed(group, "value", version, options);
    if (!options.details_only) {
        if ((value_details.type == STRING) != (seed_details.type == STRING)) {
            throw std::runtime_error("both or neither of the 'seed' and 'value' arrays should contain strings");
        }
        if (seed_dims.size() != value_details.dimensions.size()) {
            throw std::runtime_error("'seed' and 'value' arrays should have the same dimensionality");
        }

        auto ihandle = group.openGroup("index");
        auto collected = validate_subset_index_list(ihandle, seed_dims, version);
        auto expected_dims = seed_dims;
        for (auto p : collected) {
            expected_dims[p.first] = p.second;
        }

        if (!are_dimensions_equal(expected_dims, value_details.dimensions)) {
            throw std::runtime_error("'value' dimension extents are not consistent with lengths of indices in 'index'");
        }
    }

    // Promotion.
    seed_details.type = std::max(seed_details.type, value_details.type);
    return seed_details;
}

}

#endif
