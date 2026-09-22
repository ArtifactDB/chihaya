#ifndef CHIHAYA_UNARY_ARITHMETIC_HPP
#define CHIHAYA_UNARY_ARITHMETIC_HPP

#include "H5Cpp.h"
#include "ritsuko/ritsuko.hpp"

#include <stdexcept>
#include <string>

#include "utils_public.hpp"
#include "utils_type.hpp"
#include "utils_misc.hpp"
#include "utils_nary.hpp"

/**
 * @file unary_arithmetic.hpp
 * @brief Validation for delayed unary arithmetic operations.
 */

namespace chihaya {

/**
 * @param group HDF5 group representing an unary arithmetic operation.
 * @param version Version of the **chihaya** specification.
 * @param options Validation options.
 *
 * @return Details of the object after applying the arithmetic operation.
 * Otherwise, if the validation failed, an error is raised.
 */
inline ArrayDetails validate_unary_arithmetic(const H5::Group& group, const ritsuko::Version& version, Options& options) {
    auto seed_details = fetch_numeric_seed(group, "seed", version, options);

    auto method = load_scalar_string_dataset(group, "method");
    if (!options.details_only) {
        if (!is_valid_arithmetic_operation(method)) {
            throw std::runtime_error("unrecognized operation in 'method' (got '" + method + "')");
        }
    }

    auto side = load_scalar_string_dataset(group, "side");
    if (!options.details_only) {
        if (side == "none") {
            if (method != "+" && method != "-") {
                throw std::runtime_error("'side' cannot be 'none' for operation '" + method + "'");
            } 
        } else if (side != "left" && side != "right") {
            throw std::runtime_error("'side' for operation '" + method + "' should be 'left' or 'right' (got '" + side + "')");
        }
    }

    // If side = none, we set it to INTEGER to promote BOOLEANs to integer (implicit multiplication by +/-1).
    ArrayType val_type = INTEGER;

    if (side != "none") {
        auto vhandle = group.openDataSet("value");

        try {
            if (version.lt(1, 1, 0)) {
                val_type = translate_type_0_99(vhandle.getTypeClass());
            } else {
                auto type = load_scalar_string_attribute(vhandle, "type");
                val_type = translate_type_1_1(type);
                check_type_1_1(vhandle, val_type);
            }

            if (val_type != INTEGER && val_type != BOOLEAN && val_type != FLOAT) {
                throw std::runtime_error("dataset should be integer, float or boolean");
            }

            if (!options.details_only) {
                validate_missing_placeholder(vhandle, version);
        
                auto vspace = vhandle.getSpace();
                const auto ndims = vspace.getSimpleExtentNdims();
                if (ndims == 0) {
                    // scalar operation.
                } else if (ndims == 1) {
                    hsize_t extent;
                    vspace.getSimpleExtentDims(&extent);
                    check_unary_along(group, version, seed_details.dimensions, extent);
                } else { 
                    throw std::runtime_error("dataset should be scalar or 1-dimensional");
                }
            }

        } catch (std::exception& e) {
            throw std::runtime_error("failed to validate 'value'; " + std::string(e.what()));
        }
    }

    // Determining type promotion rules.
    seed_details.type = determine_arithmetic_output_type(val_type, seed_details.type, method);

    return seed_details;
}

}

#endif
