#ifndef CHIHAYA_VALIDATE_HPP
#define CHIHAYA_VALIDATE_HPP

#include "subset.hpp"
#include "combine.hpp"
#include "external.hpp"
#include "transpose.hpp"
#include "utils.hpp"

/**
 * @file validate.hpp
 *
 * @brief Main validation function.
 */

namespace chihaya {

/**
 * Validate a delayed operation/array at the specified HDF5 group.
 * 
 * @param handle Open handle to a HDF5 group corresponding to a delayed operation or array.
 * @param name Name of the group inside the file, to be used for meaningful error messages.
 * This can be set to an empty string to treat the current group as the root. 
 *
 * @return Details of the array after all delayed operations in `handle` (and its children) have been applied.
 */
inline ArrayDetails validate(const H5::Group& handle, const std::string& name) {
    auto dtype = load_string_attribute(handle, "delayed_type", " for a delayed object");
    ArrayDetails output;

    if (dtype == "array") {
        try {
            auto atype = load_string_attribute(handle, "delayed_array", " for an array");

            // Checking external.
            const std::string external_prefix = "external ";
            if (atype.size() > external_prefix.size() && atype.substr(0, external_prefix.size()) == external_prefix) {
                output = validate_external(handle, name);

            } else {
                throw std::runtime_error(std::string("unknown array type '") + atype + "' at '" + name + "'");
            }

        } catch (std::exception& e) {
            throw std::runtime_error(std::string("failed to validate delayed array at '") + name + "':\n  " + e.what());
        }

    } else if (dtype == "operation") {
        try {
            auto otype = load_string_attribute(handle, "delayed_operation", " for an operation");

            // Checking subset.
            if (otype == "subset") {
                output = validate_subset(handle, name);
            } else if (otype == "combine") {
                output = validate_combine(handle, name);
            } else if (otype == "transpose") {
                output = validate_transpose(handle, name);
            } else {
                throw std::runtime_error(std::string("unknown operation type '") + otype + "' at '" + name + "'");
            }

        } catch (std::exception& e) {
            throw std::runtime_error(std::string("failed to validate delayed operation at '") + name + "':\n  " + e.what());
        }

    } else {
        throw std::runtime_error(std::string("unknown type '") + dtype + "' at '" + name + "'");
    }

    return output;
}

/**
 * Validate a delayed operation/array at the specified HDF5 group.
 * 
 * @param path Path to a HDF5 file.
 * @param name Name of the group inside the file, to be used for meaningful error messages.
 *
 * @return Details of the array after all delayed operations in `handle` (and its children) have been applied.
 */
inline ArrayDetails validate(const std::string& path, std::string name) {
    H5::H5File handle(path, H5F_ACC_RDONLY);
    auto ghandle = handle.openGroup(name);
    return validate(ghandle, name);
}

}

#endif
