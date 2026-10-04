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
inline ArrayDetails validate_unary_arithmetic(const H5::Group& group, const ritsuko::Version& version, const Options& options) {
    ArrayDetails seed_details;
    try {
        auto shandle = group.openGroup("seed");
        seed_details = validate_numeric_seed(shandle, version, options);
    } catch (...) {
        std::throw_with_nested(std::runtime_error("failed to validate 'seed'"));
    }

    std::string method;
    try {
        auto mhandle = group.openDataSet("method");
        method = read_scalar_string_dataset(mhandle);

        if (!options.details_only) {
            if (!is_valid_arithmetic_operation(method)) {
                throw std::runtime_error("unrecognized operation '" + method + "'");
            }
        }
    } catch (...) {
        std::throw_with_nested(std::runtime_error("failed to validate 'method'"));
    }

    std::string side;
    try {
        auto shandle = group.openDataSet("side");
        side = read_scalar_string_dataset(shandle);

        if (!options.details_only) {
            if (side == "none") {
                if (method != "+" && method != "-") {
                    throw std::runtime_error("cannot be 'none' for operation '" + method + "'");
                } 
            } else if (side != "left" && side != "right") {
                throw std::runtime_error("expected 'left' or 'right' for operation '" + method + "'");
            }
        }
    } catch (...) {
        std::throw_with_nested(std::runtime_error("failed to validate 'side'"));
    }

    // If side = none, we set it to INTEGER to promote BOOLEANs to integer (implicit multiplication by +/-1).
    ArrayType val_type = INTEGER;

    if (side != "none") {
        enum Failure { VALUE, ALONG };
        Failure who_failed = VALUE;
        try {
            auto vhandle = group.openDataSet("value");

            if (version.lt(1, 1, 0)) {
                val_type = translate_type_0_99(vhandle.getTypeClass());
            } else {
                try {
                    auto thandle = vhandle.openAttribute("type");
                    auto type = read_scalar_string_attribute(thandle);
                    val_type = translate_type_1_1(type);
                    check_type_1_1(vhandle, val_type);
                } catch (...) {
                    std::throw_with_nested(std::runtime_error("failed to validate the 'type' attribute"));
                }
            }

            if (val_type != INTEGER && val_type != BOOLEAN && val_type != FLOAT) {
                throw std::runtime_error("dataset should be integer, float or boolean");
            }

            if (!options.details_only) {
                validate_missing_placeholder(vhandle, version);
        
                auto vspace = vhandle.getSpace();
                auto ndims = vspace.getSimpleExtentNdims();
                if (ndims == 0) {
                    // scalar operation.
                } else if (ndims == 1) {
                    hsize_t extent;
                    vspace.getSimpleExtentDims(&extent);
                    who_failed = ALONG;
                    auto ahandle = group.openDataSet("along");
                    check_unary_along(ahandle, version, seed_details.dimensions, extent);
                    who_failed = VALUE;
                } else { 
                    throw std::runtime_error("dataset should be scalar or 1-dimensional");
                }
            }

        } catch (...) {
            std::string desc;
            switch (who_failed) {
                case VALUE: desc = "value"; break;
                case ALONG: desc = "along"; break;
            }
            std::throw_with_nested(std::runtime_error("failed to validate '" + desc + "'"));
        }
    }

    // Determining type promotion rules.
    seed_details.type = determine_arithmetic_output_type(val_type, seed_details.type, method);

    return seed_details;
}

}

#endif
