#ifndef CHIHAYA_UNARY_MATH_HPP
#define CHIHAYA_UNARY_MATH_HPP

#include "H5Cpp.h"
#include "ritsuko/ritsuko.hpp"

#include <stdexcept>
#include <string>

#include "utils_public.hpp"
#include "utils_misc.hpp"
#include "utils_nary.hpp"

/**
 * @file unary_math.hpp
 * @brief Validation for delayed unary math operations.
 */

namespace chihaya {

/**
 * @param handle An open handle on a HDF5 group representing an unary math operation.
 * @param version Version of the **chihaya** specification.
 * @param options Validation options.
 *
 * @return Details of the object after applying the mathal operation.
 * Otherwise, if the validation failed, an error is raised.
 */
inline ArrayDetails validate_unary_math(const H5::Group& handle, const ritsuko::Version& version, Options& options) {
    auto seed_details = fetch_numeric_seed(handle, "seed", version, options);
    if (seed_details.type == STRING) {
        throw std::runtime_error("type of 'seed' should be integer, float or boolean");
    }

    // Checking the method.
    auto method = load_scalar_string_dataset(handle, "method");
    if (method == "sign") {
        seed_details.type = INTEGER;

    } else if (method == "abs") {
        seed_details.type = std::max(seed_details.type, INTEGER);

    } else if (
        method == "log1p" ||
        method == "sqrt" ||
        method == "exp" ||
        method == "expm1" ||
        method == "ceiling" ||
        method == "floor" || 
        method == "trunc" ||
        method == "sin" ||
        method == "cos" ||
        method == "tan" ||
        method == "acos" ||
        method == "asin" ||
        method == "atan" ||
        method == "sinh" ||
        method == "cosh" ||
        method == "tanh" ||
        method == "acosh" ||
        method == "asinh" ||
        method == "atanh"
    ) {
        seed_details.type = FLOAT;

    } else if (method == "log") {
        if (!options.details_only) {
            if (handle.exists("base")) {
                auto vhandle = handle.openDataSet("base");
                if (vhandle.getSpace().getSimpleExtentNdims() != 0) {
                    throw std::runtime_error("'base' should be a scalar");
                }
                if (version.lt(1, 1, 0)) {
                    if (vhandle.getTypeClass() != H5T_FLOAT) {
                        throw std::runtime_error("'base' should be a floating-point number");
                    }
                } else {
                    if (ritsuko::hdf5::exceeds_float_limit(vhandle, 64)) {
                        throw std::runtime_error("'base' should have a datatype that fits into a 64-bit float");
                    }
                }
            }
        }
        seed_details.type = FLOAT;

    } else if (method == "round" || method == "signif") {
        if (!options.details_only) {
            auto vhandle = handle.openDataSet("digits");
            if (vhandle.getSpace().getSimpleExtentNdims() != 0) {
                throw std::runtime_error("'digits' should be a scalar");
            }

            if (version.lt(1, 1, 0)) {
                if (vhandle.getTypeClass() != H5T_INTEGER) {
                    throw std::runtime_error("'digits' should be an integer");
                }
            } else {
                if (ritsuko::hdf5::exceeds_integer_limit(vhandle, 32, true)) {
                    throw std::runtime_error("'digits' should have a datatype that fits into a 32-bit signed integer");
                }
            }
        }
        seed_details.type = FLOAT;

    } else {
        throw std::runtime_error("unrecognized operation in 'method' (got '" + method + "')");
    }

    return seed_details;
}

}

#endif
