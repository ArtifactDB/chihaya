#ifndef CHIHAYA_UTILS_UNARY_HPP
#define CHIHAYA_UTILS_UNARY_HPP

#include "H5Cpp.h"
#include "ritsuko/hdf5/hdf5.hpp"

#include <stdexcept>
#include <cstdint>
#include <vector>
#include <string>

#include "utils_misc.hpp"

namespace chihaya {

namespace internal_unary {

inline std::string load_method(const H5::Group& handle) {
    auto mhandle = ritsuko::hdf5::open_dataset(handle, "method");
    if (mhandle.getSpace().getSimpleExtentNdims() != 0 || mhandle.getTypeClass() != H5T_STRING) {
        throw std::runtime_error("'method' should be a scalar string");
    }
    return ritsuko::hdf5::load_scalar_string_dataset(mhandle);
}

inline std::string load_side(const H5::Group& handle) {
    auto shandle = ritsuko::hdf5::open_dataset(handle, "side");
    if (shandle.getSpace().getSimpleExtentNdims() != 0 || shandle.getTypeClass() != H5T_STRING) {
        throw std::runtime_error("'side' should be a scalar string");
    }
    return ritsuko::hdf5::load_scalar_string_dataset(shandle);
}

inline void check_along(const H5::Group& handle, const std::vector<size_t>& seed_dimensions) {
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
