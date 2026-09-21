#ifndef CHIHAYA_UNARY_LOGIC_HPP
#define CHIHAYA_UNARY_LOGIC_HPP

#include "H5Cpp.h"
#include "ritsuko/ritsuko.hpp"

#include <stdexcept>
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
 * @param handle An open handle on a HDF5 group representing an unary logic operation.
 * @param version Version of the **chihaya** specification.
 * @param options Validation options.
 *
 * @return Details of the object after applying the logical operation.
 * Otherwise, if the validation failed, an error is raised.
 */
inline ArrayDetails validate_unary_logic(const H5::Group& handle, const ritsuko::Version& version, Options& options) {
    auto seed_details = fetch_numeric_seed(handle, "seed", version, options);

    if (!options.details_only) { 
        auto method = load_scalar_string_dataset(handle, "method");
        if (method != "!" && method != "&&" && method != "||") {
            throw std::runtime_error("unrecognized operation in 'method' (got '" + method + "')");
        }

        // Checking the sidedness.
        if (method != "!") {
            auto side = load_scalar_string_dataset(handle, "side");
            if (side != "left" && side != "right") {
                throw std::runtime_error("'side' for operation '" + method + "' should be 'left' or 'right' (got '" + side + "')");
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
                    check_unary_along(handle, version, seed_details.dimensions, extent);
                } else { 
                    throw std::runtime_error("dataset should be scalar or 1-dimensional");
                }
            } catch (std::exception& e) {
                throw std::runtime_error("failed to validate 'value'; " + std::string(e.what()));
            }
        }
    }

    seed_details.type = BOOLEAN;
    return seed_details;
}

}

#endif
