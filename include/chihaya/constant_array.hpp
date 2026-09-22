#ifndef CHIHAYA_CONSTANT_ARRAY_HPP
#define CHIHAYA_CONSTANT_ARRAY_HPP

/**
 * @file constant_array.hpp
 * @brief Constant array, stored inside the file.
 */

#include "H5Cpp.h"
#include "ritsuko/ritsuko.hpp"

#include <vector>
#include <stdexcept>
#include <string>
#include <cstdint>
#include <cstddef>

#include "utils_public.hpp"
#include "utils_type.hpp"
#include "utils_misc.hpp"
#include "utils_dimensions.hpp"

namespace chihaya {

/**
 * @param group HDF5 group representing a constant array.
 * @param version Version of the **chihaya** specification.
 * @param options Validation options.
 *
 * @return Details of the constant array.
 * Otherwise, if the validation failed, an error is raised.
 */
inline ArrayDetails validate_constant_array(const H5::Group& group, const ritsuko::Version& version, [[maybe_unused]] Options& options) {
    ArrayDetails output;

    auto dhandle = group.openDataSet("dimensions");
    auto dspace = dhandle.getSpace();
    if (dspace.getSimpleExtentNdims() != 1) {
        throw std::runtime_error("'dimensions' dataset should be 1-dimensional");
    }
    hsize_t size;
    dspace.getSimpleExtentDims(&size);
    if (size == 0) {
        throw std::runtime_error("'dimensions' dataset should have non-zero length");
    }

    if (version.lt(1, 1, 0)) {
        output.dimensions = load_non_negative_integer_vector_0_99<std::size_t>(dhandle, size);
    } else {
        if (ritsuko::hdf5::exceeds_integer_limit(dhandle, 64, false)) {
            throw std::runtime_error("datatype of 'dimensions' should fit inside a 64-bit unsigned integer");
        }
        output.dimensions = load_dimensions_from_uint64_contents<std::size_t>(dhandle, size);
    }

    auto vhandle = group.openDataSet("value");
    if (vhandle.getSpace().getSimpleExtentNdims() != 0) {
        throw std::runtime_error("'value' dataset should be a scalar");
    }

    try {
        if (version.lt(1, 1, 0)) {
            output.type = translate_type_0_99(vhandle.getTypeClass());
        } else {
            auto type = load_scalar_string_attribute(vhandle, "type");
            output.type = translate_type_1_1(type);
            if (!options.details_only) {
                check_type_1_1(vhandle, output.type);
            }
        }

        if (!options.details_only) {
            validate_missing_placeholder(vhandle, version);
            if (vhandle.getTypeClass() == H5T_STRING) {
                ritsuko::hdf5::validate_scalar_string(vhandle);
            }
        }

    } catch (std::exception& e) {
        throw std::runtime_error("failed to validate 'value'; " + std::string(e.what()));
    }

    return output;
}

}

#endif
