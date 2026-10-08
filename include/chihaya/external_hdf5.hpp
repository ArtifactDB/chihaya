#ifndef CHIHAYA_EXTERNAL_HDF5_HPP
#define CHIHAYA_EXTERNAL_HDF5_HPP

#include "H5Cpp.h"
#include "ritsuko/ritsuko.hpp"

#include <exception>
#include <stdexcept>

#include "custom_array.hpp"
#include "utils_misc.hpp"

/**
 * @file external_hdf5.hpp
 * @brief Validation for external HDF5 arrays.
 */

namespace chihaya {

/**
 * @param group HDF5 group representing an external HDF5 array.
 * @param version Version of the **chihaya** specification.
 * @param options Validation options.
 *
 * @return Details of the external HDF5 array.
 *
 * @throw H5::Exception Thrown upon HDF5 library error.
 * This may be wrapped in a `std::nested_exception`.
 * @throw std::exception Thrown upon validation failure.
 * This may be a `std::nested_exception`, in which case the nested exceptions should be extracted to obtain an informative error message.
 */
inline ArrayDetails validate_external_hdf5(const H5::Group& group, const ritsuko::Version& version, const Options& options) {
    if (version.ge(1, 1, 0)) {
        throw std::runtime_error("'external_hdf5' array type is deprecated in versions >= 1.1");
    }
    auto deets = validate_minimal_array(group, version, options);
    if (!options.details_only) {
        try {
            validate_scalar_string_dataset(group.openDataSet("file"));
        } catch (...) {
            std::throw_with_nested(std::runtime_error("failed to validate 'file'"));
        }
        try {
            validate_scalar_string_dataset(group.openDataSet("name"));
        } catch (...) {
            std::throw_with_nested(std::runtime_error("failed to validate 'name'"));
        }
    }    
    return deets;
}

}

#endif
