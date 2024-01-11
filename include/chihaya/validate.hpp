#ifndef CHIHAYA_VALIDATE_HPP
#define CHIHAYA_VALIDATE_HPP

#include "H5Cpp.h"
#include "ritsuko/ritsuko.hpp"
#include "ritsuko/hdf5/hdf5.hpp"

#include "subset.hpp"
#include "combine.hpp"
#include "transpose.hpp"

#include "dense_array.hpp"
#include "sparse_matrix.hpp"
#include "external_hdf5.hpp"
#include "custom_array.hpp"
#include "constant_array.hpp"

#include "dimnames.hpp"
#include "subset_assignment.hpp"

#include "unary_arithmetic.hpp"
#include "unary_comparison.hpp"
#include "unary_logic.hpp"
#include "unary_math.hpp"
#include "unary_special_check.hpp"

#include "binary_arithmetic.hpp"
#include "binary_comparison.hpp"
#include "binary_logic.hpp"

#include "matrix_product.hpp"

#include "utils_public.hpp"

/**
 * @file validate.hpp
 * @brief Main validation function.
 */

namespace chihaya {

/**
 * @param handle Open handle to a HDF5 group corresponding to a delayed operation or array.
 * @param version Version of the **chihaya** specification.
 *
 * @return Details of the array after all delayed operations in `handle` (and its children) have been applied.
 */
inline ArrayDetails validate(const H5::Group& handle, const ritsuko::Version& version) {
    auto dtype = ritsuko::hdf5::open_and_load_scalar_string_attribute(handle, "delayed_type");
    ArrayDetails output;

    if (dtype == "array") {
        auto atype = ritsuko::hdf5::open_and_load_scalar_string_attribute(handle, "delayed_array");

        try {
            if (atype == "dense array") {
                output = dense_array::validate(handle, version);
            } else if (atype == "sparse matrix") {
                output = sparse_matrix::validate(handle, version);
            } else if (atype == "constant array") {
                output = constant_array::validate(handle, version);
            } else if (atype.rfind("custom ", 0) != std::string::npos) {
                output = custom_array::validate(handle, version);
            } else if (atype.rfind("external hdf5 ", 0) != std::string::npos && version.lt(1, 1, 0)) {
                output = external_hdf5::validate(handle, version);
            } else {
                throw std::runtime_error("unknown array type");
            }
        } catch (std::exception& e) {
            throw std::runtime_error("failed to validate delayed array of type '" + atype + "'; " + std::string(e.what()));
        }

    } else if (dtype == "operation") {
        auto otype = ritsuko::hdf5::open_and_load_scalar_string_attribute(handle, "delayed_operation");

        try {
            if (otype == "subset") {
                output = subset::validate(handle, version);
            } else if (otype == "combine") {
                output = combine::validate(handle, version);
            } else if (otype == "transpose") {
                output = transpose::validate(handle, version);
            } else if (otype == "dimnames") {
                output = dimnames::validate(handle, version);
            } else if (otype == "subset assignment") {
                output = subset_assignment::validate(handle, version);
            } else if (otype == "unary arithmetic") {
                output = unary_arithmetic::validate(handle, version);
            } else if (otype == "unary comparison") {
                output = unary_comparison::validate(handle, version);
            } else if (otype == "unary logic") {
                output = unary_logic::validate(handle, version);
            } else if (otype == "unary math") {
                output = unary_math::validate(handle, version);
            } else if (otype == "unary special check") {
                output = unary_special_check::validate(handle, version);
            } else if (otype == "binary arithmetic") {
                output = binary_arithmetic::validate(handle, version);
            } else if (otype == "binary comparison") {
                output = binary_comparison::validate(handle, version);
            } else if (otype == "binary logic") {
                output = binary_logic::validate(handle, version);
            } else if (otype == "matrix product") {
                output = matrix_product::validate(handle, version);
            } else {
                throw std::runtime_error("unknown operation type");
            }
        } catch (std::exception& e) {
            throw std::runtime_error("failed to validate delayed operation of type '" + otype + "'; " + std::string(e.what()));
        }

    } else {
        throw std::runtime_error("unknown delayed type '" + dtype + "'");
    }

    return output;
}

/**
 * Validate a delayed operation/array at the specified HDF5 group,
 * using a version string in the `delayed_version` attribute of the `handle`.
 * This should be a string of the form `<MAJOR>.<MINOR>`.
 * For back-compatibility purposes, the string `"1.0.0"` is also allowed, corresponding to version 1.1;
 * and if `delayed_version` is missing, it defaults to `0.99`.
 * 
 * @param handle Open handle to a HDF5 group corresponding to a delayed operation or array.
 * @return Details of the array after all delayed operations in `handle` (and its children) have been applied.
 */
inline ArrayDetails validate(const H5::Group& handle) {
    ritsuko::Version version;

    if (handle.attrExists("delayed_version")) {
        auto ahandle = handle.openAttribute("delayed_version");
        if (!ritsuko::hdf5::is_utf8_string(ahandle)) {
            throw std::runtime_error("expected 'delayed_version' to use a datatype that can be represented by a UTF-8 encoded string");
        }

        auto vstring = ritsuko::hdf5::load_scalar_string_attribute(ahandle);
        if (vstring == "1.0.0") {
            version.major = 1;
        } else {
            version = ritsuko::parse_version_string(vstring.c_str(), vstring.size(), /* skip_patch = */ true);
        }
    } else {
        version.minor = 99;
    }

    return validate(handle, version);
}

/**
 * Validate a delayed operation/array at the specified HDF5 group.
 * This simply calls the `validate()` overload for a `H5::Group`.
 * 
 * @param path Path to a HDF5 file.
 * @param name Name of the group inside the file.
 *
 * @return Details of the array after all delayed operations have been applied.
 */
inline ArrayDetails validate(const std::string& path, std::string name) {
    H5::H5File handle(path, H5F_ACC_RDONLY);
    auto ghandle = handle.openGroup(name);
    return validate(ghandle);
}

}

#endif
