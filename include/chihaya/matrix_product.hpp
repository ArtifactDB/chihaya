#ifndef CHIHAYA_MATRIX_PRODUCT_HPP
#define CHIHAYA_MATRIX_PRODUCT_HPP

#include "H5Cpp.h"
#include "ritsuko/ritsuko.hpp"

#include <stdexcept>
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
inline std::pair<ArrayDetails, bool> fetch_matprod_seed(const H5::Group& handle, const std::string& target, const std::string& orientation, const ritsuko::Version& version, Options& options) {
    auto seed_details = fetch_numeric_seed(handle, target, version, options);
    if (seed_details.dimensions.size() != 2) {
        throw std::runtime_error("expected '" + target + "' to be a 2-dimensional array for a matrix product");
    }
    
    auto oristr = load_scalar_string_dataset(handle, orientation);
    if (oristr != "N" && oristr != "T") {
        throw std::runtime_error("'" + orientation + "' should be either 'N' or 'T' for a matrix product");
    }

    return std::pair<ArrayDetails, bool>(seed_details, oristr == "T");
}
/**
 * @endcond
 */

/**
 * @param handle An open handle on a HDF5 group representing a matrix product.
 * @param version Version of the **chihaya** specification.
 * @param options Validation options.
 *
 * @return Details of the matrix product.
 * Otherwise, if the validation failed, an error is raised.
 */
inline ArrayDetails validate_matrix_product(const H5::Group& handle, const ritsuko::Version& version, Options& options) {
    auto left_details = fetch_matprod_seed(handle, "left_seed", "left_orientation", version, options);
    auto right_details = fetch_matprod_seed(handle, "right_seed", "right_orientation", version, options);

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
