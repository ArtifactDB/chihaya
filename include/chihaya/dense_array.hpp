#ifndef CHIHAYA_DENSE_ARRAY_HPP
#define CHIHAYA_DENSE_ARRAY_HPP

#include "H5Cpp.h"
#include "ritsuko/ritsuko.hpp"
#include "ritsuko/hdf5/hdf5.hpp"

#include <vector>
#include <cstdint>

#include "utils_public.hpp"
#include "utils_type.hpp"
#include "utils_dimnames.hpp"

/**
 * @file dense_array.hpp
 * @brief Dense array, stored inside the file.
 */

namespace chihaya {

/**
 * @namespace chihaya::dense_array
 * @brief Namespace for dense arrays.
 */
namespace dense_array {

/**
 * @param handle An open handle on a HDF5 group representing a dense array.
 * @param version Version of the **chihaya** specification.
 *
 * @return Details of the dense array.
 * Otherwise, if the validation failed, an error is raised.
 */
inline ArrayDetails validate(const H5::Group& handle, const ritsuko::Version& version) {
    ArrayDetails output;

    {
        auto dhandle = ritsuko::hdf5::open_dataset(handle, "data");
        auto dspace = dhandle.getSpace();
        auto ndims = dspace.getSimpleExtentNdims();
        if (ndims == 0) {
            throw std::runtime_error("'data' should have non-zero dimensions for a dense array");
        }

        std::vector<hsize_t> dims(ndims);
        dspace.getSimpleExtentDims(dims.data());
        output.dimensions.insert(output.dimensions.end(), dims.begin(), dims.end());

        if (is_version_at_or_below(version, 1, 0)) {
            output.type = translate_type_0_0(dhandle.getTypeClass());
            if (is_boolean(dhandle)) {
                output.type = BOOLEAN;
            }
        } else {
            auto type = internal_type::fetch_delayed_type(dhandle);
            internal_type::check_type_1_1(dhandle, type);
            output.type = internal_type::translate_type_1_1(type);
        }

        validate_missing_placeholder(dhandle, version);
    }

    {
        auto nhandle = ritsuko::hdf5::open_dataset(handle, "native");
        if (nhandle.getSpace().getSimpleExtentNdims() != 0) {
            throw std::runtime_error("'native' should be a scalar");
        }

        bool native;
        if (is_version_at_or_below(version, 1, 0)) {
            if (nhandle.getTypeClass() != H5T_INTEGER) {
                throw std::runtime_error("'native' should have an integer datatype");
            }
            int native_tmp;
            nhandle.read(&native_tmp, H5::PredType::NATIVE_INT);
            native = native_tmp;
        } else {
            if (ritsuko::hdf5::exceeds_integer_limit(nhandle, 8, false)) {
                throw std::runtime_error("'native' should have a datatype that fits into an 8-bit signed integer");
            }
            int8_t native_tmp;
            nhandle.read(&native_tmp, H5::PredType::NATIVE_INT8);
            native = native_tmp;
        }

        if (!native) {
            std::reverse(output.dimensions.begin(), output.dimensions.end());
        }
    }

    if (handle.exists("dimnames")) {
        internal_dimnames::validate(handle, output.dimensions, version);
    }

    return output;
}

}

}

#endif
