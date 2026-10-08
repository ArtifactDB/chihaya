#ifndef CHIHAYA_UNARY_COMPARISON_HPP
#define CHIHAYA_UNARY_COMPARISON_HPP

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
 * @file unary_comparison.hpp
 * @brief Validation for delayed unary comparisons.
 */

namespace chihaya {

/**
 * @param group HDF5 group representing an unary comparison operation.
 * @param version Version of the **chihaya** specification.
 * @param options Validation options.
 *
 * @return Details of the object after applying the comparison operation.
 * Otherwise, if the validation failed, an exception is thrown.
 * This exception may be nested.
 */
inline ArrayDetails validate_unary_comparison(const H5::Group& group, const ritsuko::Version& version, const Options& options) {
    ArrayDetails seed_details;
    try {
        auto shandle = group.openGroup("seed");
        seed_details = validate(shandle, version, options);
    } catch (...) {
        std::throw_with_nested(std::runtime_error("failed to validate 'seed'"));
    }

    if (!options.details_only) {
        std::string method;
        try {
            auto mhandle = group.openDataSet("method");
            method = read_scalar_string_dataset(mhandle);
            if (!is_valid_comparison_operation(method)) {
                throw std::runtime_error("unrecognized operation '" + method + "'");
            }
        } catch (...) {
            std::throw_with_nested(std::runtime_error("failed to validate 'method'"));
        }

        std::string side;
        try {
            auto shandle = group.openDataSet("side");
            auto side = read_scalar_string_dataset(shandle);
            if (side != "left" && side != "right") {
                throw std::runtime_error("expected either 'left' or 'right'");
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
            if ((val_type == STRING) != (seed_details.type == STRING)) {
                throw std::runtime_error("both or neither of 'seed' and 'value' should contain strings");
            }

            validate_missing_placeholder(vhandle, version);

            auto ndims = vhandle.getSpace().getSimpleExtentNdims();
            if (ndims == 0) { // scalar operation.
                if (vhandle.getTypeClass() == H5T_STRING) {
                    ritsuko::hdf5::validate_scalar_string(vhandle);
                }

            } else if (ndims == 1) {
                hsize_t extent;
                vhandle.getSpace().getSimpleExtentDims(&extent);
                who_failed = ALONG;
                auto ahandle = group.openDataSet("along");
                check_unary_along(ahandle, version, seed_details.dimensions, extent);
                who_failed = VALUE;

                if (vhandle.getTypeClass() == H5T_STRING) {
                    ritsuko::hdf5::validate_1d_strings(
                        vhandle,
                        extent, 
                        [&]{
                            ritsuko::hdf5::Validate1dStringsOptions opt;
                            opt.contiguous_chunk_size = options.contiguous_chunk_size;
                            return opt;
                        }()
                    );
                }

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

    seed_details.type = BOOLEAN;
    return seed_details;
}

}

#endif
