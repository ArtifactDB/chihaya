#ifndef CHIHAYA_DIMNAMES_HPP
#define CHIHAYA_DIMNAMES_HPP

#include "H5Cpp.h"
#include "ritsuko/ritsuko.hpp"
#include "ritsuko/hdf5/hdf5.hpp"
#include <stdexcept>
#include "utils_list.hpp"
#include "utils_public.hpp"

/**
 * @file dimnames.hpp
 * @brief Validation for delayed dimnames assignment.
 */

namespace chihaya {

/**
 * @cond
 */
inline ArrayDetails validate(const H5::Group&, const ritsuko::Version&);
/**
 * @endcond
 */

namespace dimnames {

/**
 * @param handle An open handle on a HDF5 group representing a dimnames assignment operation.
 * @param version Version of the **chihaya** specification.
 *
 * @return Details of the object after assigning dimnames.
 * Otherwise, if the validation failed, an error is raised.
 */
inline ArrayDetails validate(const H5::Group& handle, const ritsuko::Version& version) {
    auto ghandle = ritsuko::hdf5::open_group(handle, "seed");

    ArrayDetails seed_details;
    try {
        seed_details = ::chihaya::validate(ghandle, version);
    } catch (std::exception& e) {
        throw std::runtime_error("failed to validate 'seed'; " + std::string(e.what()));
    }

    if (!handle.exists("dimnames")) {
        throw std::runtime_error("expected a 'dimnames' group");
    }
    internal_dimnames::validate(handle, seed_details.dimensions, version);

    return seed_details;
}

}

}

#endif
