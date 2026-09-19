#ifndef CHIHAYA_EXTERNAL_HDF5_HPP
#define CHIHAYA_EXTERNAL_HDF5_HPP

#include "H5Cpp.h"
#include "ritsuko/ritsuko.hpp"

#include "custom_array.hpp"
#include "utils_misc.hpp"

/**
 * @file external_hdf5.hpp
 * @brief Validation for external HDF5 arrays.
 */

namespace chihaya {

/**
 * @param handle An open handle on a HDF5 group representing an external HDF5 array.
 * @param version Version of the **chihaya** specification.
 * @param options Validation options.
 *
 * @return Details of the external HDF5 array.
 * Otherwise, if the validation failed, an error is raised.
 */
inline ArrayDetails validate_external_hdf5(const H5::Group& handle, const ritsuko::Version& version, Options& options) {
    if (version.ge(1, 1, 0)) {
        throw std::runtime_error("'external_hdf5' array type is deprecated in versions >= 1.1");
    }
    auto deets = validate_minimal_array(handle, version, options);
    if (!options.details_only) {
        safe_open_scalar_string_dataset(handle, "file");
        safe_open_scalar_string_dataset(handle, "name");
    }
    return deets;
}

}

#endif
