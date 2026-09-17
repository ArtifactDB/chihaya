#ifndef CHIHAYA_CUSTOM_ARRAY_HPP
#define CHIHAYA_CUSTOM_ARRAY_HPP

#include "H5Cpp.h"
#include "ritsuko/ritsuko.hpp"

#include "minimal_array.hpp"

/**
 * @file custom_array.hpp
 *
 * @brief Validation for custom third-party arrays.
 */

namespace chihaya {

/**
 * @param handle An open handle on a HDF5 group representing an external array.
 * @param version Version of the **chihaya** specification.
 * @param options Validation options.
 *
 * @return Details of the custom array.
 * Otherwise, if the validation failed, an error is raised.
 */
inline ArrayDetails validate_custom_array(const H5::Group& handle, const ritsuko::Version& version, Options& options) {
    return validate_minimal_array(handle, version, options);
}

}

#endif
