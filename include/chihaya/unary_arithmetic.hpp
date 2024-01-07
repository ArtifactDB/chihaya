#ifndef CHIHAYA_UNARY_ARITHMETIC_HPP
#define CHIHAYA_UNARY_ARITHMETIC_HPP

#include "H5Cpp.h"
#include "ritsuko/ritsuko.hpp"
#include "ritsuko/hdf5/hdf5.hpp"

#include <stdexcept>
#include <string>

#include "utils_public.hpp"
#include "utils_type.hpp"
#include "utils_misc.hpp"
#include "utils_arithmetic.hpp"

/**
 * @file unary_arithmetic.hpp
 * @brief Validation for delayed unary arithmetic operations.
 */

namespace chihaya {

/**
 * @cond
 */
inline ArrayDetails validate(const H5::Group&, const ritsuko::Version&);
/**
 * @endcond
 */

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

    std::string method;
    {
        auto mhandle = ritsuko::hdf5::open_dataset(handle, "method");
        if (mhandle.getSpace().getSimpleExtentNdims() != 0 || mhandle.getTypeClass() != H5T_STRING) {
            throw std::runtime_error("'method' should be a scalar string for an unary arithmetic operation");
        }

        method = ritsuko::hdf5::load_scalar_string_dataset(mhandle);
        if (!internal_arithmetic::is_valid_operation(method)) {
            throw std::runtime_error("unrecognized 'method' (" + method + ")");
        }
    }

    std::string side;
    {
        auto shandle = ritsuko::hdf5::open_dataset(handle, "side");
        if (shandle.getSpace().getSimpleExtentNdims() != 0 || shandle.getTypeClass() != H5T_STRING) {
            throw std::runtime_error("'side' should be a scalar string for an unary arithmetic operation");
        }

        side = ritsuko::hdf5::load_scalar_string_dataset(shandle);
        if (side == "none") {
            if (method != "+" && method != "-") {
                throw std::runtime_error("'side' cannot be 'none' for method '" + method + "'");
            } 
        } else if (side != "left" && side != "right") {
            throw std::runtime_error(std::string("unrecognized side '" + side + "'");
        }
    }

    // If side = none, we set it to INTEGER to promote BOOLEANs to integer (i.e., multiplication by +/-1).
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
            auto type  = internal_type::fetch_delayed_type(dhandle);
            internal_type::check_numeric_type_1_1(dhandle, type);
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

            // Checking 'along'.
            uint64_t along;
            auto ahandle = ritsuko::hdf5::open_dataset(handle, "along");
            if (ahandle.getSpace().getSimpleExtentNdims() != 0) {
                throw std::runtime_error("'along' should be a scalar dataset");
            }

            if (internal_misc::is_version_at_or_below(version, 1, 0)) {
                if (ahandle.getTypeClass() != H5T_INTEGER) {
                    throw std::runtime_error("'along' should be an integer dataset");
                }
                int along_tmp; 
                ahandle.read(&along_tmp, H5::PredType::NATIVE_INT);
                if (along_tmp < 0) {
                    throw std::runtime_error("'along' should be non-negative");
                }
                along = along_tmp;
            } else {
                if (ritsuko::hdf5::exceeds_integer_limits(ahandle, 64, false)) {
                    throw std::runtime_error("'along' should have a datatype that fits in a 64-bit unsigned integer");
                }
                ahandle.read(&along, H5::PredType::NATIVE_UINT64);
            }

            if (static_cast<size_t>(along) >= seed_details.dimensions.size()) {
                throw std::runtime_error("'along' should be less than the seed dimensionality");
            }

            if (extent != seed_details.dimensions[along]) {
                throw std::runtime_error("length of 'value' dataset should be equal to the dimension specified in 'along'");
            }
        } else { 
            throw std::runtime_error("'value' dataset should be scalar or 1-dimensional for an unary arithmetic operation");
        }
    }

    // Determining type promotion rules.
    seed_details.type = internal_arithmetic::determine_output_type(min_type, seed_details.type, method);

    return seed_details;
}

}

#endif
