#ifndef TAKANE_UTILS_DIMNAMES_HPP
#define TAKANE_UTILS_DIMNAMES_HPP

#include "H5Cpp.h"
#include "ritsuko/hdf5/hdf5.hpp"
#include "ritsuko/ritsuko.hpp"
#include <string>
#include <stdexcept>
#include "utils_list.hpp"

namespace chihaya {

namespace internal_dimnames {

template<class V>
void validate(const H5::Group& handle, const V& dimensions, const ritsuko::Version& version) {
    auto ghandle = ritsuko::open_group(handle, "dimnames");

    internal_list::ListDetails list_params;
    try {
        list_params = internal_list::validate(ghandle, version);
    } catch (std::exception& e) {
        throw std::runtime_error("failed to load 'dimnames' list");
    }

    if (list_params.length != dimensions.size()) {
        throw std::runtime_error("length of 'dimnames' list should be equal to seed dimensionality");
    }

    for (const auto& p : list_params.present) {
        auto current = ritsuko::hdf5::open_dataset(ghandle, p.second.c_str());
        if (current.getSpace().getSimpleExtentNdims() != 1 || current.getTypeClass() != H5T_STRING) {
            throw std::runtime_error("each entry of 'dimnames' should be a 1-dimensional string dataset");
        }
        if (ritsuko::hdf5::get_1d_length(current, false) != static_cast<hsize_t>(dimensions[p.first])) {
            throw std::runtime_error("each entry of 'dimnames' should have length equal to the extent of its corresponding dimension");
        }
    }
}

}

}

#endif
