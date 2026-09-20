#ifndef CHIHAYA_UTILS_DIMENSIONS_HPP
#define CHIHAYA_UTILS_DIMENSIONS_HPP

#include "H5Cpp.h"

#include "ritsuko/ritsuko.hpp"
#include "sanisizer/sanisizer.hpp"

#include <stdexcept>
#include <type_traits>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <cassert>

#include "utils_type.hpp"
#include "utils_list.hpp"

namespace chihaya {

template<class Vector_>
bool are_dimensions_equal(const Vector_& left, const Vector_& right) {
    const auto nleft = left.size();
    if (nleft != right.size()) {
        return false;
    }
    for (I<decltype(nleft)> i = 0; i < nleft; ++i) {
        if (left[i] != right[i]) {
            return false;
        }
    }
    return true;
}

template<typename Output_, typename Ndims_>
std::vector<Output_> load_dimensions_from_uint64_contents(const H5::DataSet& handle, Ndims_ ndims) {
    assert(handle.getSpace().getSimpleExtentNdims() == 1);
    assert(!ritsuko::hdf5::exceeds_integer_limit(handle, 64, false));

    auto output = sanisizer::create<std::vector<Output_> >(ndims);
    if constexpr(std::is_same<std::uint64_t, Output_>::value) {
        // Avoid a copy if we can.
        handle.read(output.data(), H5::PredType::NATIVE_UINT64);
    } else {
        auto tmp = sanisizer::create<std::vector<std::uint64_t> >(ndims);
        handle.read(tmp.data(), H5::PredType::NATIVE_UINT64);
        for (I<Ndims_> d = 0; d < ndims; ++d) {
            output[d] = sanisizer::cast<Output_>(tmp[d]);
        }
    }
    return output;
}

inline std::uint64_t load_along(const H5::Group& handle, const ritsuko::Version& version) {
    auto ahandle = handle.openDataSet("along");
    if (ahandle.getSpace().getSimpleExtentNdims() != 0) {
        throw std::runtime_error("'along' dataset should be scalar");
    }
    if (version.lt(1, 1, 0)) {
        return load_non_negative_integer_scalar_0_99<std::uint64_t>(ahandle);
    } else {
        if (ritsuko::hdf5::exceeds_integer_limit(ahandle, 64, false)) {
            throw std::runtime_error("'along' dataset should use a datatype that fits in a 64-bit unsigned integer");
        }
        std::uint64_t val;
        ahandle.read(&val, H5::PredType::NATIVE_UINT64);
        return val;
    }
}

inline void validate_dimnames_internal(const H5::Group& handle, const std::vector<std::size_t>& dimensions, const ritsuko::Version& version) try {
    auto ghandle = handle.openGroup("dimnames");
    auto list_params = validate_list(ghandle, version);

    if (!sanisizer::is_equal(list_params.length, dimensions.size())) {
        throw std::runtime_error("length of 'dimnames' list should be equal to seed dimensionality");
    }

    for (const auto& p : list_params.present) {
        auto current = ghandle.openDataSet(p.second);
        auto cspace = current.getSpace();
        if (cspace.getSimpleExtentNdims() != 1) {
            throw std::runtime_error("each entry of 'dimnames' should be a 1-dimensional string dataset");
        }
        if (!ritsuko::hdf5::is_utf8_string(current)) {
            throw std::runtime_error("each entry of 'dimnames' should use a datatype that can be represented by UTF-8 strings");
        }

        hsize_t len;
        cspace.getSimpleExtentDims(&len);
        if (!sanisizer::is_equal(len, dimensions[p.first])) {
            throw std::runtime_error("each entry of 'dimnames' should have length equal to the extent of its corresponding dimension");
        }

        ritsuko::hdf5::validate_1d_strings(current, len);
    }
} catch (std::exception& e) {
    throw std::runtime_error("failed to validate the 'dimnames'; " + std::string(e.what()));
}

}

#endif
