#ifndef CHIHAYA_COMBINE_HPP
#define CHIHAYA_COMBINE_HPP

#include "H5Cpp.h"
#include "ritsuko/ritsuko.hpp"
#include "sanisizer/sanisizer.hpp"

#include <stdexcept>
#include <vector>
#include <string>
#include <cstddef>

#include "utils_public.hpp"
#include "utils_misc.hpp"
#include "utils_list.hpp"
#include "utils_dimensions.hpp"

/**
 * @file combine.hpp
 * @brief Validation for delayed combining operations.
 */

namespace chihaya {

/**
 * @param group HDF5 group representing a combining operation.
 * @param version Version of the **chihaya** specification.
 * @param options Validation options.
 * 
 * @return Details of the combined object.
 * Otherwise, if the validation failed, an error is raised.
 */
inline ArrayDetails validate_combine(const H5::Group& group, const ritsuko::Version& version, Options& options) {
    const auto along = load_along(group, version);

    const auto shandle = group.openGroup("seeds");
    ListDetails list_params;
    try {
        list_params = validate_list(shandle, version);
    } catch (std::exception& e) {
        throw std::runtime_error("failed to load 'seeds' list; " + std::string(e.what()));
    }
    if (list_params.present.size() != list_params.length) {
        throw std::runtime_error("missing elements in the 'seeds' list");
    }

    std::vector<std::size_t> dimensions;
    ArrayType type = BOOLEAN;
    bool first = true;
    I<decltype(list_params.length)> num_strings = 0;

    for (auto& p : list_params.present) {
        ArrayDetails cur_seed;
        try {
            cur_seed = fetch_seed(shandle, p.second, version, options);
        } catch (std::exception& e) {
            throw std::runtime_error("failed to validate 'seeds/" + p.second + "'; " + std::string(e.what()));
        }

        if (first) {
            type = cur_seed.type;
            dimensions = std::move(cur_seed.dimensions);
            if (sanisizer::is_greater_than_or_equal(along, dimensions.size())) {
                throw std::runtime_error("'along' should be less than the seed dimensionality");
            }
            first = false;

        } else {
            if (type < cur_seed.type) {
                type = cur_seed.type;
            }
            const auto ndims = dimensions.size();
            if (ndims != cur_seed.dimensions.size()) {
                throw std::runtime_error("dimensionality mismatch between seeds");
            }
            for (I<decltype(ndims)> d = 0; d < ndims; ++d) {
                if (sanisizer::is_equal(d, along)) {
                    dimensions[d] = sanisizer::sum<std::size_t>(dimensions[d], cur_seed.dimensions[d]);
                } else if (dimensions[d] != cur_seed.dimensions[d]) {
                    throw std::runtime_error("inconsistent dimension extents between seeds");
                }
            }
        }

        num_strings += (cur_seed.type == STRING);
    }

    if (num_strings != 0 && num_strings != list_params.length) {
        throw std::runtime_error("either none or all of the arrays to be combined should contain strings");
    }

    return ArrayDetails(type, std::move(dimensions));
}

}

#endif
