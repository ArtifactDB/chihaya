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
 * @param group HDF5 group representing an unary math operation.
 * @param version Version of the **chihaya** specification.
 * @param options Validation options.
 *
 * @return Details of the object after applying the mathal operation.
 * Otherwise, if the validation failed, an error is raised.
 */
inline ArrayDetails validate_unary_math(const H5::Group& group, const ritsuko::Version& version, const Options& options) {
    ArrayDetails seed_details;
    try {
        auto shandle = group.openGroup("seed");
        seed_details = validate(shandle, version, options);
    } catch (...) {
        wrap_error(std::current_exception(), "failed to validate 'seed'");
    }
    if (seed_details.type == STRING) {
        throw std::runtime_error("type of 'seed' should be integer, float or boolean");
    }

    std::string method;
    try {
        auto mhandle = group.openDataSet("method");
        method = read_scalar_string_dataset(mhandle);
    } catch (...) {
        wrap_error(std::current_exception(), "failed to validate 'method'");
    }

    if (method == "sign") {
        seed_details.type = INTEGER;

    } else if (method == "abs") {
        seed_details.type = std::max(seed_details.type, INTEGER);

    } else if (method == "log") {
        if (!options.details_only) {
            if (group.exists("base")) {
                try {
                    auto vhandle = group.openDataSet("base");
                    if (vhandle.getSpace().getSimpleExtentNdims() != 0) {
                        throw std::runtime_error("expected a scalar dataset");
                    }
                    if (version.lt(1, 1, 0)) {
                        if (vhandle.getTypeClass() != H5T_FLOAT) {
                            throw std::runtime_error("expected a floating-point number");
                        }
                    } else {
                        if (ritsuko::hdf5::exceeds_float_limit(vhandle, 64)) {
                            throw std::runtime_error("expected a datatype that fits into a 64-bit float");
                        }
                    }
                } catch (...) {
                    wrap_error(std::current_exception(), "failed to validate 'base'");
                }
            }
        }
        seed_details.type = FLOAT;

    } else if (method == "round" || method == "signif") {
        if (!options.details_only) {
            try {
                auto vhandle = group.openDataSet("digits");
                if (vhandle.getSpace().getSimpleExtentNdims() != 0) {
                    throw std::runtime_error("'expected a scalar dataset");
                }

                if (version.lt(1, 1, 0)) {
                    if (vhandle.getTypeClass() != H5T_INTEGER) {
                        throw std::runtime_error("expected an integer");
                    }
                } else {
                    if (ritsuko::hdf5::exceeds_integer_limit(vhandle, 32, true)) {
                        throw std::runtime_error("expected a datatype that fits into a 32-bit signed integer");
                    }
                }
            } catch (...) {
                wrap_error(std::current_exception(), "failed to validate 'digits'");
            }
        }
        seed_details.type = FLOAT;

    } else if (is_other_math(method)) {
        seed_details.type = FLOAT;

    } else {
        throw std::runtime_error("unrecognized operation '" + method + "' in 'method'");
    }

    return seed_details;
}

}

#endif
