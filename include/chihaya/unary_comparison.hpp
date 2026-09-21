#ifndef CHIHAYA_UNARY_COMPARISON_HPP
#define CHIHAYA_UNARY_COMPARISON_HPP

#include "H5Cpp.h"
#include "ritsuko/ritsuko.hpp"

#include <stdexcept>
#include <string>

#include "utils_public.hpp"
#include "utils_misc.hpp"
#include "utils_type.hpp"
#include "utils_nary.hpp"

/**
 * @file unary_comparison.hpp
 * @brief Validation for delayed unary comparisons.
 */

namespace chihaya {

/**
 * @param handle An open handle on a HDF5 group representing an unary comparison operation.
 * @param version Version of the **chihaya** specification.
 * @param options Validation options.
 *
 * @return Details of the object after applying the comparison operation.
 * Otherwise, if the validation failed, an error is raised.
 */
inline ArrayDetails validate_unary_comparison(const H5::Group& handle, const ritsuko::Version& version, Options& options) {
    auto seed_details = fetch_seed(handle, "seed", version, options);

    if (!options.details_only) {
        auto method = load_scalar_string_dataset(handle, "method");
        if (!is_valid_comparison_operation(method)) {
            throw std::runtime_error("unrecognized operation in 'method' (got '" + method + "')");
        }

        auto side = load_scalar_string_dataset(handle, "side");
        if (side != "left" && side != "right") {
            throw std::runtime_error("'side' should be either 'left' or 'right' (got '" + side + "')");
        }

        // Checking the value.
        auto vhandle = handle.openDataSet("value");
        try {
            ArrayType val_type;
            if (version.lt(1, 1, 0)) {
                val_type = translate_type_0_99(vhandle.getTypeClass());
            } else {
                auto type = load_scalar_string_attribute(vhandle, "type");
                val_type = translate_type_1_1(type);
                check_type_1_1(vhandle, val_type);
            }
            if ((val_type == STRING) != (seed_details.type == STRING)) {
                throw std::runtime_error("both or neither of 'seed' and 'value' should contain strings");
            }

            validate_missing_placeholder(vhandle, version);

            size_t ndims = vhandle.getSpace().getSimpleExtentNdims();
            if (ndims == 0) { // scalar operation.
                if (vhandle.getTypeClass() == H5T_STRING) {
                    ritsuko::hdf5::validate_scalar_string(vhandle);
                }

            } else if (ndims == 1) {
                hsize_t extent;
                vhandle.getSpace().getSimpleExtentDims(&extent);
                check_unary_along(handle, version, seed_details.dimensions, extent);
                if (vhandle.getTypeClass() == H5T_STRING) {
                    ritsuko::hdf5::validate_1d_strings(vhandle, extent);
                }

            } else { 
                throw std::runtime_error("dataset should be scalar or 1-dimensional");
            }

        } catch (std::exception& e) {
            throw std::runtime_error("failed to validate 'value'; " + std::string(e.what()));
        }
    }

    seed_details.type = BOOLEAN;
    return seed_details;
}

}

#endif
