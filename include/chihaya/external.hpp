#ifndef CHIHAYA_EXTERNAL_HPP
#define CHIHAYA_EXTERNAL_HPP

#include <vector>
#include <stdexcept>
#include <cstdint>
#include "H5Cpp.h"

/**
 * @file external.hpp
 *
 * @brief Validation for external arrays.
 */

namespace chihaya {

/**
 * Validate an external array, where the only provided information is its dimensions.
 *
 * @param handle An open handle on a HDF5 group representing an external array.
 * @param name Name of the group inside the file.
 *
 * @return Vector containing the dimensions of the external array.
 *
 * Each external array is represented as a HDF5 group with the following attributes:
 *
 * - `delayed_type` should be a scalar string `"array"`.
 * - `delayed_array` should be any scalar string that starts with `"external "` (note the space).
 *
 * Inside the group, we expect a 1-dimensional integer dataset named `dimensions`.
 * This stores the dimensionality of the external array and should only contain non-negative values.
 * 
 * The external array concept allows us to embed references to external resources, e.g., arrays held in databases.
 * This avoids making a redundant copy of a large arrays when we only want to preserve the delayed operations.
 * Of course, it is assumed that clients will know how to retrieve specific resources from the remotes.
 */
inline std::vector<size_t> validate_external(const H5::Group& handle, const std::string& name) {
    if (!handle.exists("dimensions") || handle.childObjType("dimensions") != H5O_TYPE_DATASET) {
        throw std::runtime_error("expected 'dimensions' dataset for an external array"); 
    }

    auto dhandle = handle.openDataSet("dimensions");
    auto dspace = dhandle.getSpace();
    if (dspace.getSimpleExtentNdims() != 1 || dhandle.getTypeClass() != H5T_INTEGER) {
        throw std::runtime_error("'dimensions' should be a 1-dimensional integer dataset for an external array");
    }

    hsize_t len;
    dspace.getSimpleExtentDims(&len);

    std::vector<int64_t> dimensions(len);
    dhandle.read(dimensions.data(), H5::PredType::NATIVE_INT64);
    for (auto d : dimensions) {
        if (d < 0) {
            throw std::runtime_error("elements in 'dimensions' should be non-negative for an external array");
        }
    } 

    return std::vector<size_t>(dimensions.begin(), dimensions.end());
}

}

#endif
