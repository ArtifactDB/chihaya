#ifndef CHIHAYA_MATRIX_PRODUCT_HPP
#define CHIHAYA_MATRIX_PRODUCT_HPP

#include "H5Cpp.h"
#include "ritsuko/ritsuko.hpp"

#include <stdexcept>
#include <exception>
#include <string>

#include "utils_public.hpp"
#include "utils_misc.hpp"
#include "utils_nary.hpp"

/**
 * @file matrix_product.hpp
 * @brief Validation for delayed matrix products.
 */

namespace chihaya {

/**
 * @cond
 */
inline std::pair<ArrayDetails, bool> fetch_matprod_seed(
    const H5::Group& group,
    const std::string& target,
    const std::string& orientation,
    const ritsuko::Version& version,
    const Options& options
) {
    ArrayDetails seed_details;
    try {
        auto thandle = group.openGroup(target);
        seed_details = validate_numeric_seed(thandle, version, options);
    } catch (...) {
        std::throw_with_nested(std::runtime_error("failed to validate '" + target + "'"));
    }

    if (seed_details.dimensions.size() != 2) {
        throw std::runtime_error("expected '" + target + "' to be a 2-dimensional array");
    }

    std::string oristr;
    try {
        auto ohandle = group.openDataSet(orientation);
        oristr = read_scalar_string_dataset(ohandle);
        if (oristr != "N" && oristr != "T") {
            throw std::runtime_error("expected either 'N' or 'T'");
        }
    } catch (...) {
        std::throw_with_nested(std::runtime_error("failed to validate '" + orientation + "'"));
    }

    return std::pair<ArrayDetails, bool>(seed_details, oristr == "T");
}
/**
 * @endcond
 */

/**
 * @param group HDF5 group representing a matrix product.
 * @param version Version of the **chihaya** specification.
 * @param options Validation options.
 *
 * @return Details of the matrix product.
 * Otherwise, if the validation failed, an exception is thrown.
 * This exception may be nested.
 */
inline ArrayDetails validate_matrix_product(const H5::Group& group, const ritsuko::Version& version, const Options& options) {
    auto left_details = fetch_matprod_seed(group, "left_seed", "left_orientation", version, options);
    auto right_details = fetch_matprod_seed(group, "right_seed", "right_orientation", version, options);

    ArrayDetails output;
    output.dimensions.resize(2);
    auto& nrow = output.dimensions[0];
    auto& ncol = output.dimensions[1];
    I<decltype(nrow)> common, common2;

    if (left_details.second) {
        nrow = left_details.first.dimensions[1];
        common = left_details.first.dimensions[0];
    } else {
        nrow = left_details.first.dimensions[0];
        common = left_details.first.dimensions[1];
    }

    if (right_details.second) {
        ncol = right_details.first.dimensions[0];
        common2 = right_details.first.dimensions[1];
    } else {
        ncol = right_details.first.dimensions[1];
        common2 = right_details.first.dimensions[0];
    }

    if (!options.details_only) {
        if (common != common2) {
            throw std::runtime_error("inconsistent common dimensions (" + std::to_string(common) + " vs " + std::to_string(common2) + ")");
        }
    }

    if (left_details.first.type == FLOAT || right_details.first.type == FLOAT) {
        output.type = FLOAT;
    } else {
        output.type = INTEGER;
    }

    return output;
}

}

#endif
