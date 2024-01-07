#ifndef CHIHAYA_BINARY_ARITHMETIC_HPP
#define CHIHAYA_BINARY_ARITHMETIC_HPP

#include "H5Cpp.h"
#include "ritsuko/hdf5/hdf5.hpp"

#include <stdexcept>
#include <vector>
#include <string>

#include "utils_public.hpp"
#include "utils_arithmetic.hpp"
#include "utils_misc.hpp"

/**
 * @file binary_arithmetic.hpp
 * @brief Validation for delayed binary arithmetic operations.
 */

namespace chihaya {

/**
 * @cond
 */
inline ArrayDetails validate(const H5::Group&, const Version&);
/**
 * @endcond
 */

/**
 * @namespace chihaya::binary_arithmetic
 * @brief Namespace for delayed binary arithmetic operations.
 */
namespace binary_arithmetic {

/**
 * @param handle An open handle on a HDF5 group representing a binary arithmetic operation.
 * @param version Version of the **chihaya** specification.
 *
 * @return Details of the object after applying the arithmetic operation.
 * Otherwise, if the validation failed, an error is raised.
 */
inline ArrayDetails validate(const H5::Group& handle, const Version& version) {
    auto left_details = internal_arithmetic::fetch_seed(handle, "left", version);
    auto right_details = internal_arithmetic::fetch_seed(handle, "right", version);

    bool okay = internal_misc::are_dimensions_equal(left_details.dimensions, right_details.dimensions);
    if (!okay) {
        throw std::runtime_error("'left' and 'right' should have the same dimensions");
    }

    auto mhandle = ritsuko::hdf5::open_dataset(handle, "method");
    if (mhandle.getSpace().getSimpleExtentNdims() != 0 || mhandle.getTypeClass() != H5T_STRING) {
        throw std::runtime_error("'method' should be a scalar string");
    }

    auto method = ritsuko::hdf5::load_scalar_string_dataset(mhandle);
    if (!internal_arithmetic::is_valid_operation(method)) {
        throw std::runtime_error("unrecognized 'method' (" + method + ")");
    }

    left_details.type = internal_arithmeric::determine_output_type(left_details.type, right_details.type, method);
    return left_details;
}

}

}

#endif
