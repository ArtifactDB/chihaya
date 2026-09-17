#ifndef CHIHAYA_MINIMAL_ARRAY_HPP
#define CHIHAYA_MINIMAL_ARRAY_HPP

#include "H5Cpp.h"
#include "ritsuko/ritsuko.hpp"

#include <vector>
#include <stdexcept>
#include <cstdint>
#include <algorithm>

#include "utils_public.hpp"
#include "utils_type.hpp"
#include "utils_misc.hpp"
#include "utils_dimensions.hpp"

namespace chihaya {

inline ArrayDetails validate_minimal_array(const H5::Group& handle, const ritsuko::Version& version, [[maybe_unused]] Options& options) {
    ArrayDetails output;

    auto dhandle = handle.openDataSet("dimensions");
    auto dspace = dhandle.getSpace();
    if (dspace.getSimpleExtentNdims() != 1) {
        throw std::runtime_error("'dimensions' dataset should be 1-dimensional");
    }
    hsize_t len;
    dspace.getSimpleExtentDims(&len);

    if (version.lt(1, 1, 0)) {
        output.dimensions = load_non_negative_integer_vector_0_99<std::size_t>(dhandle, len);
    } else {
        if (ritsuko::hdf5::exceeds_integer_limit(dhandle, 64, false)) {
            throw std::runtime_error("datatype of 'dimensions' should fit in a 64-bit unsigned integer");
        }
        output.dimensions = load_dimensions_from_uint64_contents<std::size_t>(dhandle, len);
    }

    auto type = load_scalar_string_dataset(handle, "type");
    if (type == "BOOLEAN") {
        output.atype = BOOLEAN;
    } else if (type == "INTEGER") {
        output.atype = INTEGER;
    } else if (type == "FLOAT") {
        output.atype = FLOAT;
    } else if (type == "STRING") {
        output.atype = STRING;
    } else {
        throw std::runtime_error("unknown 'type' (" + type + ")");
    }

    return output;
}

}

#endif
