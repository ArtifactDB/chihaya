#ifndef CHIHAYA_BINARY_COMPARISON_HPP
#define CHIHAYA_BINARY_COMPARISON_HPP

#include "H5Cpp.h"
#include "ritsuko/hdf5/hdf5.hpp"

#include <stdexcept>
#include <vector>
#include <algorithm>

#include "utils_misc.hpp"
#include "utils_comparison.hpp"

/**
 * @file binary_comparison.hpp
 * @brief Validation for delayed binary comparisons.
 */

namespace chihaya {

/**
 * @cond
 */
ArrayDetails validate(const H5::Group&, const Version&);
/**
 * @endcond
 */

/**
 * @namespace chihaya::binary_comparison
 * @brief Namespace for delayed binary comparisons.
 */
namespace binary_comparison {

/**
 * @param handle An open handle on a HDF5 group representing a binary comparison.
 * @param version Version of the **chihaya** specification.
 *
 * @return Details of the object after applying the comparison operation.
 * Otherwise, if the validation failed, an error is raised.
 */
inline ArrayDetails validate(const H5::Group& handle, const Version& version) {
    auto left_details = ::chihaya::validate(ritsuko::hdf5::open_group(handle, "left"), version);
    auto right_details = ::chihaya::validate(ritsuko::hdf5::open_group(handle, "right"), version);

    bool okay = internal_misc::are_dimensions_equal(left_details.dimensions, right_details.dimensions);
    if (!okay) {
        throw std::runtime_error("'left' and 'right' should have the same dimensions");
    }

    if ((left_details.type == STRING) != (right_details.type == STRING)) {
        throw std::runtime_error("both or neither of 'left' and 'right' should contain strings");
    }

    auto mhandle = ritsuko::hdf5::open_dataset(handle, "method");
    if (mhandle.getSpace().getSimpleExtentNdims() != 0 || mhandle.getTypeClass() != H5T_STRING) {
        throw std::runtime_error("'method' should be a scalar string");
    }

    auto method = ritsuko::hdf5::load_scalar_string_dataset(mhandle);
    if (!internal_comparison::is_valid_operation(method)) {
        throw std::runtime_error("unrecognized 'method' (" + method + ")");
    }

    left_details.type = BOOLEAN;
    return left_details;
}

}

}

#endif
