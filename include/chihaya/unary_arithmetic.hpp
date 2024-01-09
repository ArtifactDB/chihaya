#ifndef CHIHAYA_UNARY_ARITHMETIC_HPP
#define CHIHAYA_UNARY_ARITHMETIC_HPP

#include "H5Cpp.h"
#include "ritsuko/ritsuko.hpp"
#include "ritsuko/hdf5/hdf5.hpp"

#include <stdexcept>

#include "utils_public.hpp"
#include "utils_unary.hpp"
#include "utils_type.hpp"
#include "utils_misc.hpp"
#include "utils_arithmetic.hpp"

/**
 * @file unary_arithmetic.hpp
 * @brief Validation for delayed unary arithmetic operations.
 */

namespace chihaya {

/**
 * @namespace chihaya::unary_arithmetic
 * @brief Namespace for delayed unary arithmetic operations.
 */
namespace unary_arithmetic {

/**
 * @param handle An open handle on a HDF5 group representing an unary arithmetic operation.
 * @param version Version of the **chihaya** specification.
 *
 * @return Details of the object after applying the arithmetic operation.
 * Otherwise, if the validation failed, an error is raised.
 */
inline ArrayDetails validate(const H5::Group& handle, const ritsuko::Version& version) {
    auto seed_details = internal_arithmetic::fetch_seed(handle, "seed", version);

    auto method = internal_unary::load_method(handle);
    if (!internal_arithmetic::is_valid_operation(method)) {
        throw std::runtime_error("unrecognized 'method' (" + method + ")");
    }

    auto side = internal_unary::load_side(handle);
    if (side == "none") {
        if (method != "+" && method != "-") {
            throw std::runtime_error("'side' cannot be 'none' for method '" + method + "'");
        } 
    } else if (side != "left" && side != "right") {
        throw std::runtime_error("unrecognized side '" + side + "'");
    }

    // If side = none, we set it to INTEGER to promote BOOLEANs to integer (implicit multiplication by +/-1).
    ArrayType min_type = INTEGER;

    if (side != "none") {
        auto vhandle = ritsuko::hdf5::open_dataset(handle, "value");
        
        if (internal_misc::is_version_at_or_below(version, 1, 0)) {
            if (vhandle.getTypeClass() == H5T_STRING) {
                throw std::runtime_error("'value' dataset should be numeric or boolean for an unary arithmetic operation");
            } else if (vhandle.getTypeClass() == H5T_FLOAT) {
                min_type = FLOAT;
            }
        } else {
            auto type = internal_type::fetch_data_type(vhandle);
            internal_type::check_numeric_type_1_1(vhandle, type);
            min_type = internal_type::translate_type_1_1(type);
        }

        internal_misc::validate_missing_placeholder(vhandle, version);
    
        auto vspace = vhandle.getSpace();
        size_t ndims = vspace.getSimpleExtentNdims();
        if (ndims == 0) {
            // scalar operation.
        } else if (ndims == 1) {
            hsize_t extent;
            vspace.getSimpleExtentDims(&extent);
            internal_unary::check_along(handle, version, seed_details.dimensions, extent);
        } else { 
            throw std::runtime_error("'value' dataset should be scalar or 1-dimensional for an unary arithmetic operation");
        }
    }

    // Determining type promotion rules.
    seed_details.type = internal_arithmetic::determine_output_type(min_type, seed_details.type, method);

    return seed_details;
}

}

}

#endif
