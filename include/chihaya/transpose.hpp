#ifndef CHIHAYA_TRANSPOSE_HPP
#define CHIHAYA_TRANSPOSE_HPP

#include "H5Cpp.h"
#include "ritsuko/ritsuko.hpp"

#include <stdexcept>
#include <algorithm>
#include <vector>
#include <cstdint>
#include <cstddef>

#include "utils_public.hpp"
#include "utils_misc.hpp"
#include "utils_type.hpp"

/**
 * @file transpose.hpp
 * @brief Validation for delayed transposition.
 */

namespace chihaya {

/**
 * @cond
 */
template<typename Perm_, typename Ndim_>
std::vector<std::size_t> check_permutation(const H5::DataSet& phandle, Ndim_ plen, const std::vector<size_t>& input_dimensions, bool details_only) {
    if (!sanisizer::is_equal(plen, input_dimensions.size())) {
        throw std::runtime_error("length of 'permutation' should match dimensionality of 'seed'");
    }

    auto permutation = sanisizer::create<std::vector<Perm_> >(plen);
    phandle.read(permutation.data(), ritsuko::hdf5::as_numeric_datatype<Perm_>());

    auto new_dimensions = sanisizer::create<std::vector<std::size_t> >(plen);
    for (I<decltype(plen)> p = 0; p < plen; ++p) {
        const auto current = permutation[p];
        if (current < 0) {
            throw std::runtime_error("'permutation' should contain non-negative indices");
        }
        if (sanisizer::is_greater_than_or_equal(current, plen)) {
            throw std::runtime_error("'permutation' contains out-of-bounds indices");
        }
        new_dimensions[p] = input_dimensions[permutation[p]];
    }

    if (!details_only) {
        std::sort(permutation.begin(), permutation.end());
        for (I<decltype(plen)> p = 0; p < plen; ++p) {
            if (!sanisizer::is_equal(p, permutation[p])) {
                throw std::runtime_error("indices in 'permutation' should be unique");
            }
        }
    }

    return new_dimensions;
}
/**
 * @endcond
 */

/**
 * @param group HDF5 group representing a transposition.
 * @param version Version of the **chihaya** specification.
 * @param options Validation options.
 *
 * @return Details of the transposed object.
 * Otherwise, if the validation failed, an error is raised.
 */
inline ArrayDetails validate_transpose(const H5::Group& group, const ritsuko::Version& version, Options& options) {
    auto seed_details = fetch_seed(group, "seed", version, options);

    auto phandle = group.openDataSet("permutation");
    auto pspace = phandle.getSpace();
    if (pspace.getSimpleExtentNdims() != 1) {
        throw std::runtime_error("'permutation' should be a 1-dimensional dataset");
    }
    hsize_t plen;
    pspace.getSimpleExtentDims(&plen);

    if (version.lt(1, 1, 0)) {
        // Older versions didn't actually specify the integer type, so we just check we can load it into an 'int' of any type.
        if (!ritsuko::hdf5::exceeds_integer_limit(phandle, 64, true)) {
            seed_details.dimensions = check_permutation<std::int64_t>(phandle, plen, seed_details.dimensions, options.details_only);
        } else if (!ritsuko::hdf5::exceeds_integer_limit(phandle, 64, false)) {
            seed_details.dimensions = check_permutation<std::uint64_t>(phandle, plen, seed_details.dimensions, options.details_only);
        } else {
            throw create_integer_error_0_99(ritsuko::hdf5::get_name(phandle), phandle.getTypeClass());
        }
    } else {
        if (ritsuko::hdf5::exceeds_integer_limit(phandle, 64, false)) {
            throw std::runtime_error("'permutation' should have a datatype that can be represented by a 64-bit unsigned integer");
        }
        seed_details.dimensions = check_permutation<std::uint64_t>(phandle, plen, seed_details.dimensions, options.details_only);
    }

    return seed_details;
}

}

#endif
