#ifndef CHIHAYA_DENSE_ARRAY_HPP
#define CHIHAYA_DENSE_ARRAY_HPP

#include "H5Cpp.h"
#include <vector>
#include "utils.hpp"
#include "dimnames.hpp"

/**
 * @file dense_array.hpp
 *
 * @brief Dense array, stored inside the file.
 */

namespace chihaya {

/**
 * Validate a dense array in a HDF5 file.
 *
 * @param handle An open handle on a HDF5 group representing a dense array.
 * @param name Name of the group inside the file.
 *
 * @return Details of the dense array.
 * Otherwise, if the validation failed, an error is raised.
 * 
 * A dense array is represented as a HDF5 group with the following attributes:
 *
 * - `delayed_type` should be a scalar string `"array"`.
 * - `delayed_operation` should be a scalar string `"dense array"`.
 *
 * Inside the group, we expect:
 *
 * - A `data` dataset, containing the array data.
 *   This should have a non-zero number of dimensions (i.e., not scalar) and contain integers, floats or strings.
 *   The exact type representation is left to the implementation.
 *
 * The group may also contain:
 *
 * - A `dimnames` group, representing a list (see `ListDetails`) of length equal to the number of dimensions in the `seed`.
 *   Each child entry corresponds to a dimension of `seed` and contains the names along that dimension.
 *   Missing entries indicate that no names are attached to its dimension.
 *   Each (non-missing) entry should be a 1-dimensional string dataset of length equal to the extent of its dimension.
 *   The exact string representation is left to the implementation.
 */
inline ArrayDetails validate_dense_array(const H5::Group& handle, const std::string& name) {
    // Check for a 'data' group.
    if (!handle.exists("data") || handle.childObjType("data") != H5O_TYPE_DATASET) {
        throw std::runtime_error("'data' should be a dataset for a dense array");
    }

    auto dhandle = handle.openDataSet("data");
    auto ndims = dhandle.getSpace().getSimpleExtentNdims();
    if (ndims == 0) {
        throw std::runtime_error("'data' should have non-zero dimensions for a dense array");
    }

    ArrayDetails output;
    std::vector<hsize_t> dims(ndims);
    dhandle.getSpace().getSimpleExtentDims(dims.data());
    output.dimensions.insert(output.dimensions.end(), dims.begin(), dims.end());

    auto cls = dhandle.getTypeClass();
    if (cls == H5T_INTEGER) {
        output.type = INTEGER;
    } else if (cls == H5T_FLOAT) {
        output.type = FLOAT;
    } else if (cls == H5T_STRING) {
        output.type = STRING;
    } else {
        throw std::runtime_error("unrecognized type of 'data' for a dense array");
    }

    // Validating dimnames.
    if (handle.exists("dimnames")) {
        validate_dimnames(handle, output.dimensions, "dense array");
    }

    return output;
}

}

#endif
