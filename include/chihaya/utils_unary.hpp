#ifndef CHIHAYA_UTILS_UNARY_HPP
#define CHIHAYA_UTILS_UNARY_HPP

#include "H5Cpp.h"
#include "ritsuko/hdf5/hdf5.hpp"
#include "ritsuko/ritsuko.hpp"

#include <stdexcept>
#include <cstdint>
#include <vector>
#include <string>

#include "utils_misc.hpp"

namespace chihaya {

inline ArrayDetails validate(const H5::Group& handle, const ritsuko::Version&);

namespace internal_unary {

inline std::string load_method(const H5::Group& handle) {
    auto mhandle = ritsuko::hdf5::open_dataset(handle, "method");
    if (!ritsuko::hdf5::is_scalar(mhandle) || mhandle.getTypeClass() != H5T_STRING) {
        throw std::runtime_error("'method' should be a scalar string");
    }
    return ritsuko::hdf5::load_scalar_string_dataset(mhandle);
}

inline std::string load_side(const H5::Group& handle) {
    auto shandle = ritsuko::hdf5::open_dataset(handle, "side");
    if (!ritsuko::hdf5::is_scalar(shandle) || shandle.getTypeClass() != H5T_STRING) {
        throw std::runtime_error("'side' should be a scalar string");
    }
    return ritsuko::hdf5::load_scalar_string_dataset(shandle);
}

inline void check_along(const H5::Group& handle, const ritsuko::Version& version, const std::vector<size_t>& seed_dimensions, size_t extent) {
    size_t along = internal_misc::load_along(handle, version);

    if (static_cast<size_t>(along) >= seed_dimensions.size()) {
        throw std::runtime_error("'along' should be less than the seed dimensionality");
    }

    if (extent != seed_details.dimensions[along]) {
        throw std::runtime_error("length of 'value' dataset should be equal to the dimension specified in 'along'");
    }
}

}

}

#endif
