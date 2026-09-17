#ifndef CHIHAYA_EXTERNAL_HDF5_HPP
#define CHIHAYA_EXTERNAL_HDF5_HPP

#include "H5Cpp.h"
#include "ritsuko/ritsuko.hpp"

#include <stdexcept>

#include "minimal_array.hpp"

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
    auto deets = validate_minimal_array(handle, version, options);

    if (!options.details_only) {
        auto fhandle = handle.openDataSet("file");
        if (fhandle.getSpace().getSimpleExtentNdims() != 0) {
            throw std::runtime_error("'file' dataset should be scalar");
        }
        if (!ritsuko::hdf5::is_utf8_string(fhandle)) {
            throw std::runtime_error("'file' dataset should use a datatype that can be represented by a UTF-8 encoded string");
        }

        auto nhandle = handle.openDataSet("name");
        if (nhandle.getSpace().getSimpleExtentNdims() != 0) {
            throw std::runtime_error("'name' dataset should be scalar");
        }
        if (!ritsuko::hdf5::is_utf8_string(nhandle)) {
            throw std::runtime_error("'name' dataset should use a datatype that can be represented by a UTF-8 encoded string");
        }
    }

    return deets;
}

}

#endif
