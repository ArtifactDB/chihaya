#ifndef CHIHAYA_DIMNAMES_HPP
#define CHIHAYA_DIMNAMES_HPP

#include "H5Cpp.h"
#include <stdexcept>
#include <vector>
#include "list.hpp"
#include "utils.hpp"

namespace chihaya {

/**
 * @cond
 */
inline ArrayDetails validate(const H5::Group& handle, const std::string&);
/**
 * @endcond
 */

/**
 * Validate delayed dimnames assignment in a HDF5 file.
 *
 * @param handle An open handle on a HDF5 group representing a dimnames assignment operation.
 * @param name Name of the group inside the file.
 *
 * @return Details of the array after dimnames assignment.
 * Otherwise, if the validation failed, an error is raised.
 * 
 * A delayed subsetting operation is represented as a HDF5 group with the following attributes:
 *
 * - `delayed_type` should be a scalar string `"operation"`.
 * - `delayed_operation` should be a scalar string `"dimnames"`.
 *
 * Inside the group, we expect:
 *
 * - A `seed` group.
 *   Dimnames are to be assigned to this seed, which may be an array or anothed delayed operation.
 *   The `seed` group handle is passed to `validate()` to check its contents recursively and to retrieve the dimensions.
 * - A `dimnames` group.
 *   This is expected to be a list (see `ListDetails`) of length equal to the number of dimensions in the `seed`.
 *   Each entry is named after a dimension of `seed` using 0-based indexing, i.e., entry `"0"` corresponds to the first dimension.
 *   Entries may be missing to indicate that no names are attached to its dimension.
 *   Each (non-missing) entry should be a 1-dimensional string dataset of length equal to the extent of its dimension.
 */
inline ArrayDetails validate_dimnames(const H5::Group& handle, const std::string& name) {
    if (!handle.exists("seed") || handle.childObjType("seed") != H5O_TYPE_GROUP) {
        throw std::runtime_error("expected 'seed' group for a dimnames assignment operation");
    }
    auto seed_details = validate(handle.openGroup("seed"), name + "/seed");

    if (!handle.exists("dimnames") || handle.childObjType("dimnames") != H5O_TYPE_GROUP) {
        throw std::runtime_error("expected 'dimnames' group for a dimnames assignment operation"); 
    }

    auto dhandle = handle.openGroup("dimnames");
    ListDetails list_params;
    
    try {
        list_params = validate_list(dhandle);
    } catch (std::exception& e) {
        throw std::runtime_error(std::string("failed to load 'dimnames' list for a dimnames assignment operation:\n  ") + e.what());
    }

    if (list_params.length != seed_details.dimensions.size()) {
        throw std::runtime_error("length of 'dimnames' list should be equal to seed dimensionality");
    }

    for (const auto& p : list_params.present) {
        if (dhandle.childObjType(p.second) != H5O_TYPE_DATASET) {
            throw std::runtime_error("each entry of 'dimnames' should be a dataset");
        }

        auto current = dhandle.openDataSet(p.second);
        if (current.getSpace().getSimpleExtentNdims() != 1 || current.getTypeClass() != H5T_STRING) {
            throw std::runtime_error("each entry of 'dimnames' should be a 1-dimensional string dataset");
        }

        hsize_t dim;
        current.getSpace().getSimpleExtentDims(&dim);
        if (dim != static_cast<hsize_t>(seed_details.dimensions[p.first])) {
            throw std::runtime_error("each entry of 'dimnames' should have length equal to the extent of its corresponding dimension");
        }
    }

    return seed_details;
}

}

#endif
