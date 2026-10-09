#ifndef CHIHAYA_VALIDATE_HPP
#define CHIHAYA_VALIDATE_HPP

#include "H5Cpp.h"
#include "ritsuko/ritsuko.hpp"

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

#include <string>
#include <stdexcept>
#include <exception>
#include <unordered_map>
#include <functional>

/**
 * @file validate.hpp
 * @brief Main validation function.
 */

namespace chihaya {

/**
 * @cond
 */
inline auto default_operation_registry() {
    ValidateRegistry registry;
    registry["subset"] = validate_subset;
    registry["combine"] = validate_combine;
    registry["transpose"] = validate_transpose;
    registry["dimnames"] = validate_dimnames;
    registry["subset assignment"] = validate_subset_assignment;
    registry["unary arithmetic"] = validate_unary_arithmetic;
    registry["unary comparison"] = validate_unary_comparison;
    registry["unary logic"] = validate_unary_logic;
    registry["unary math"] = validate_unary_math;
    registry["unary special check"] = validate_unary_special_check;
    registry["binary arithmetic"] = validate_binary_arithmetic;
    registry["binary comparison"] = validate_binary_comparison;
    registry["binary logic"] = validate_binary_logic;
    registry["matrix product"] = validate_matrix_product;
    return registry;
}

inline auto default_array_registry() {
    ValidateRegistry registry;
    registry["dense array"] = validate_dense_array;
    registry["sparse matrix"] = validate_sparse_matrix;
    registry["constant array"] = validate_constant_array;
    return registry;
}
/**
 * @endcond
 */

/**
 * Validate a HDF5 group representing a delayed oepration or array.
 * For operations, this function will first search `Options::operation_validate_registry` for an available validation function.
 * For arrays, this function will first search `Options::array_validate_registry` for an available validation function.
 *
 * @param group HDF5 group representing a delayed operation or array.
 * @param version Version of the **chihaya** specification.
 * @param options Validation options, possibly containing custom validation functions.
 *
 * @return Details of the array after all delayed operations in `group` (and its children) have been applied.
 *
 * @throw H5::Exception Thrown upon HDF5 library error.
 * This may be wrapped in a `std::nested_exception`.
 * @throw std::exception Thrown upon validation failure.
 * This may be a `std::nested_exception`, in which case the nested exceptions should be extracted to obtain an informative error message.
 */
inline ArrayDetails validate(const H5::Group& group, const ritsuko::Version& version, const Options& options) {
    std::string dtype;
    try {
        auto ahandle = group.openAttribute("delayed_type");
        dtype = read_scalar_string_attribute(ahandle);
    } catch (...) {
        std::throw_with_nested(std::runtime_error("failed to validate the 'delayed_type' attribute"));
    }

    ArrayDetails output;
    if (dtype == "array") {
        std::string atype;
        try {
            auto ahandle = group.openAttribute("delayed_array");
            atype = read_scalar_string_attribute(ahandle);
        } catch (...) {
            std::throw_with_nested(std::runtime_error("failed to validate the 'delayed_array' attribute"));
        }

        const auto& custom = options.array_validate_registry;
        auto cit = custom.find(atype);
        if (cit != custom.end()) {
            try {
                output = (cit->second)(group, version, options);
            } catch (...) {
                std::throw_with_nested(std::runtime_error("failed to validate delayed array of type '" + atype + "'"));
            }

        } else {
            static const auto global = default_array_registry();
            auto git = global.find(atype);
            if (git != global.end()) {
                try {
                    output = (git->second)(group, version, options);
                } catch (...) {
                    std::throw_with_nested(std::runtime_error("failed to validate delayed array of type '" + atype + "'"));
                }

            } else if (atype.rfind("custom ", 0) != std::string::npos) {
                try {
                    output = validate_custom_array(group, version, options);
                } catch (...) {
                    std::throw_with_nested(std::runtime_error("failed to validate delayed array of type '" + atype + "'"));
                }

            } else if (atype.rfind("external hdf5 ", 0) != std::string::npos && version.lt(1, 1, 0)) {
                try {
                    output = validate_external_hdf5(group, version, options);
                } catch (...) {
                    std::throw_with_nested(std::runtime_error("failed to validate delayed array of type '" + atype + "'"));
                }

            } else {
                throw std::runtime_error("unknown array type '" + atype + "'");
            }
        }

    } else if (dtype == "operation") {
        std::string otype;
        try {
            auto ohandle = group.openAttribute("delayed_operation");
            otype = read_scalar_string_attribute(ohandle);
        } catch (...) {
            std::throw_with_nested(std::runtime_error("failed to validate the 'delayed_operation' attribute"));
        }

        const auto& custom = options.operation_validate_registry;
        auto cit = custom.find(otype);
        if (cit != custom.end()) {
            try {
                output = (cit->second)(group, version, options);
            } catch (...) {
                std::throw_with_nested(std::runtime_error("failed to validate delayed operation of type '" + otype + "'"));
            }

        } else {
            static const auto global = default_operation_registry();
            auto git = global.find(otype);
            if (git != global.end()) {
                try {
                    output = (git->second)(group, version, options);
                } catch (...) {
                    std::throw_with_nested(std::runtime_error("failed to validate delayed operation of type '" + otype + "'"));
                }

            } else {
                throw std::runtime_error("unknown operation type '" + otype + "'");
            }
        }

    } else {
        throw std::runtime_error("unknown delayed type '" + dtype + "'");
    }

    return output;
}

/**
 * The version is taken from the `delayed_version` attribute of the `group`.
 * This should be a version string of the form `<MAJOR>.<MINOR>`.
 * For back-compatibility purposes, the string `"1.0.0"` is also allowed, corresponding to version 1.0;
 * and if `delayed_version` is missing, it defaults to `0.99`.
 *
 * @param group HDF5 group corresponding to a delayed operation or array.
 *
 * @return Version of the **chihaya** specification.
 */
inline ritsuko::Version extract_version(const H5::Group& group) {
    ritsuko::Version version;

    if (group.attrExists("delayed_version")) {
        try {
            auto vhandle = group.openAttribute("delayed_version");
            auto vstring = read_scalar_string_attribute(vhandle);
            if (vstring == "1.0.0") {
                version.major = 1;
            } else {
                version = ritsuko::parse_version_string(vstring.c_str(), vstring.size(), /* skip_patch = */ true);
            }
        } catch (...) {
            std::throw_with_nested(std::runtime_error("failed to validate the 'delayed_version' attribute"));
        }
    } else {
        version.minor = 99;
    }

    return version;
}

/**
 * Overload of `validate()` that extracts the version from `group` via `extract_version()`.
 * 
 * @param group HDF5 group representing a delayed operation or array.
 * @param options Validation options, see `validate()` for details.
 *
 * @return Details of the array after all delayed operations in `group` (and its children) have been applied.
 *
 * @throw H5::Exception Thrown upon HDF5 library error.
 * This may be wrapped in a `std::nested_exception`.
 * @throw std::exception Thrown upon validation failure.
 * This may be a `std::nested_exception`, in which case the nested exceptions should be extracted to obtain an informative error message.
 */
inline ArrayDetails validate(const H5::Group& group, const Options& options) {
    return validate(group, extract_version(group), options);
}

/**
 * Overload of `validate()` that accepts a file path and group name.
 * 
 * @param path Path to a HDF5 file.
 * @param name Name of the HDF5 group inside the file representing a delayed operation or array.
 * @param options Validation options, see `validate()` for details.
 *
 * @return Details of the array after all delayed operations have been applied.
 *
 * @throw H5::Exception Thrown upon HDF5 library error.
 * This may be wrapped in a `std::nested_exception`.
 * @throw std::exception Thrown upon validation failure.
 * This may be a `std::nested_exception`, in which case the nested exceptions should be extracted to obtain an informative error message.
 */
inline ArrayDetails validate(const std::string& path, const std::string& name, const Options& options) {
    try {
        H5::H5File handle(path, H5F_ACC_RDONLY);
        auto ghandle = handle.openGroup(name);
        return validate(ghandle, options);
    } catch (...) {
        std::throw_with_nested(std::runtime_error("failed to validate '" + name + "' in '" + path + "'"));
    }
}

/**
 * Overload of `validate()` that accepts a file path and group name, and uses the default `Options`. 
 * 
 * @param path Path to a HDF5 file.
 * @param name Name of the HDF5 group inside the file representing a delayed operation or array.
 *
 * @return Details of the array after all delayed operations have been applied.
 *
 * @throw H5::Exception Thrown upon HDF5 library error.
 * This may be wrapped in a `std::nested_exception`.
 * @throw std::exception Thrown upon validation failure.
 * This may be a `std::nested_exception`, in which case the nested exceptions should be extracted to obtain an informative error message.
 */
inline ArrayDetails validate(const std::string& path, const std::string& name) {
    return validate(path, name, {});
}

}

#endif
