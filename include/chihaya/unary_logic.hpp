#ifndef CHIHAYA_UNARY_LOGIC_HPP
#define CHIHAYA_UNARY_LOGIC_HPP

#include "H5Cpp.h"
#include "ritsuko/ritsuko.hpp"

#include <stdexcept>
#include <exception>
#include <string>

#include "utils_public.hpp"
#include "utils_misc.hpp"
#include "utils_type.hpp"
#include "utils_nary.hpp"

/**
 * @file unary_logic.hpp
 *
 * @brief Validation for delayed unary logic operations.
 */

namespace chihaya {

/**
 * @param group HDF5 group representing an unary logic operation.
 * @param version Version of the **chihaya** specification.
 * @param options Validation options.
 *
 * @return Details of the object after applying the logical operation.
 * Otherwise, if the validation failed, an exception is thrown.
 * This exception may be nested.
 */
inline ArrayDetails validate_unary_logic(const H5::Group& group, const ritsuko::Version& version, const Options& options) {
    ArrayDetails seed_details;
    try {
        auto shandle = group.openGroup("seed");
        seed_details = validate_numeric_seed(shandle, version, options);
    } catch (...) {
        std::throw_with_nested(std::runtime_error("failed to validate 'seed'"));
    }

    if (!options.details_only) { 
        std::string method;
        try {
            auto mhandle = group.openDataSet("method");
            method = read_scalar_string_dataset(mhandle);
            if (method != "!" && method != "&&" && method != "||") {
                throw std::runtime_error("unrecognized operation '" + method + "'");
            }
        } catch (...) {
            std::throw_with_nested(std::runtime_error("failed to validate 'method'"));
        }

        // Checking the sidedness.
        if (method != "!") {
            std::string side;
            try {
                auto shandle = group.openDataSet("side");
                auto side = read_scalar_string_dataset(shandle);
                if (side != "left" && side != "right") {
                    throw std::runtime_error("expected 'left' or 'right' for operation '" + method + "'");
                }
            } catch (...) {
                std::throw_with_nested(std::runtime_error("failed to validate 'side'"));
            }

            enum Failure { VALUE, ALONG };
            Failure who_failed = VALUE;
            try {
                auto vhandle = group.openDataSet("value");

                ArrayType val_type;
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
                if (val_type == STRING) {
                    throw std::runtime_error("dataset should be integer, float or boolean");
                }

                validate_missing_placeholder(vhandle, version);

                auto vspace = vhandle.getSpace();
                const auto ndims = vspace.getSimpleExtentNdims();
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

            } catch (...) {
                std::string desc;
                switch (who_failed) {
                    case VALUE: desc = "value"; break;
                    case ALONG: desc = "along"; break;
                }
                std::throw_with_nested(std::runtime_error("failed to validate '" + desc + "'"));
            }
        }
    }

    seed_details.type = BOOLEAN;
    return seed_details;
}

}

#endif
