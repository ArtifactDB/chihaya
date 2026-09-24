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
    registry["subset"] = [](const H5::Group& h, const ritsuko::Version& v, const Options& o) -> ArrayDetails { return validate_subset(h, v, o); };
    registry["combine"] = [](const H5::Group& h, const ritsuko::Version& v, const Options& o) -> ArrayDetails { return validate_combine(h, v, o); };
    registry["transpose"] = [](const H5::Group& h, const ritsuko::Version& v, const Options& o) -> ArrayDetails { return validate_transpose(h, v, o); };
    registry["dimnames"] = [](const H5::Group& h, const ritsuko::Version& v, const Options& o) -> ArrayDetails { return validate_dimnames(h, v, o); };
    registry["subset assignment"] = [](const H5::Group& h, const ritsuko::Version& v, const Options& o) -> ArrayDetails { return validate_subset_assignment(h, v, o); };
    registry["unary arithmetic"] = [](const H5::Group& h, const ritsuko::Version& v, const Options& o) -> ArrayDetails { return validate_unary_arithmetic(h, v, o); };
    registry["unary comparison"] = [](const H5::Group& h, const ritsuko::Version& v, const Options& o) -> ArrayDetails { return validate_unary_comparison(h, v, o); };
    registry["unary logic"] = [](const H5::Group& h, const ritsuko::Version& v, const Options& o) -> ArrayDetails { return validate_unary_logic(h, v, o); };
    registry["unary math"] = [](const H5::Group& h, const ritsuko::Version& v, const Options& o) -> ArrayDetails { return validate_unary_math(h, v, o); };
    registry["unary special check"] = [](const H5::Group& h, const ritsuko::Version& v, const Options& o) -> ArrayDetails { return validate_unary_special_check(h, v, o); };
    registry["binary arithmetic"] = [](const H5::Group& h, const ritsuko::Version& v, const Options& o) -> ArrayDetails { return validate_binary_arithmetic(h, v, o); };
    registry["binary comparison"] = [](const H5::Group& h, const ritsuko::Version& v, const Options& o) -> ArrayDetails { return validate_binary_comparison(h, v, o); };
    registry["binary logic"] = [](const H5::Group& h, const ritsuko::Version& v, const Options& o) -> ArrayDetails { return validate_binary_logic(h, v, o); };
    registry["matrix product"] = [](const H5::Group& h, const ritsuko::Version& v, const Options& o) -> ArrayDetails { return validate_matrix_product(h, v, o); };
    return registry;
}

inline auto default_array_registry() {
    ValidateRegistry registry;
    registry["dense array"] = [](const H5::Group& h, const ritsuko::Version& v, const Options& o) -> ArrayDetails { return validate_dense_array(h, v, o); };
    registry["sparse matrix"] = [](const H5::Group& h, const ritsuko::Version& v, const Options& o) -> ArrayDetails { return validate_sparse_matrix(h, v, o); };
    registry["constant array"] = [](const H5::Group& h, const ritsuko::Version& v, const Options& o) -> ArrayDetails { return validate_constant_array(h, v, o); };
    return registry;
}
/**
 * @endcond
 */

/**
 * Validate a HDF5 group representing a delayed oepration or array.
 * For operations, this function will first search `options.custom_operation_validate_registry` for an available validation function.
 * For arrays, this function will first search `options.custom_array_validate_registry` for an available validation function.
 *
 * @param group HDF5 group representing a delayed operation or array.
 * @param version Version of the **chihaya** specification.
 * @param options Validation options, possibly containing custom validation functions.
 *
 * @return Details of the array after all delayed operations in `group` (and its children) have been applied.
 */
inline ArrayDetails validate(const H5::Group& group, const ritsuko::Version& version, const Options& options) {
    auto dtype = load_scalar_string_attribute(group, "delayed_type");
    ArrayDetails output;

    if (dtype == "array") {
        auto atype = load_scalar_string_attribute(group, "delayed_array");

        const auto& custom = options.array_validate_registry;
        auto cit = custom.find(atype);
        if (cit != custom.end()) {
            try {
                output = (cit->second)(group, version, options);
            } catch (std::exception& e) {
                throw std::runtime_error("failed to validate delayed array of type '" + atype + "'; " + std::string(e.what()));
            }

        } else {
            static const auto global = default_array_registry();
            auto git = global.find(atype);
            if (git != global.end()) {
                try {
                    output = (git->second)(group, version, options);
                } catch (std::exception& e) {
                    throw std::runtime_error("failed to validate delayed array of type '" + atype + "'; " + std::string(e.what()));
                }
            } else if (atype.rfind("custom ", 0) != std::string::npos) {
                try {
                    output = validate_custom_array(group, version, options);
                } catch (std::exception& e) {
                    throw std::runtime_error("failed to validate delayed array of type '" + atype + "'; " + std::string(e.what()));
                }
            } else if (atype.rfind("external hdf5 ", 0) != std::string::npos && version.lt(1, 1, 0)) {
                try {
                    output = validate_external_hdf5(group, version, options);
                } catch (std::exception& e) {
                    throw std::runtime_error("failed to validate delayed array of type '" + atype + "'; " + std::string(e.what()));
                }
            } else {
                throw std::runtime_error("unknown array type '" + atype + "'");
            }
        }

    } else if (dtype == "operation") {
        auto otype = load_scalar_string_attribute(group, "delayed_operation");

        const auto& custom = options.operation_validate_registry;
        auto cit = custom.find(otype);
        if (cit != custom.end()) {
            try {
                output = (cit->second)(group, version, options);
            } catch (std::exception& e) {
                throw std::runtime_error("failed to validate delayed operation of type '" + otype + "'; " + std::string(e.what()));
            }

        } else {
            static const auto global = default_operation_registry();
            auto git = global.find(otype);
            if (git != global.end()) {
                try {
                    output = (git->second)(group, version, options);
                } catch (std::exception& e) {
                    throw std::runtime_error("failed to validate delayed operation of type '" + otype + "'; " + std::string(e.what()));
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
 * @return Version of the **chihaya** specification.
 */
inline ritsuko::Version extract_version(const H5::Group& group) {
    ritsuko::Version version;

    if (group.attrExists("delayed_version")) {
        auto vstring = load_scalar_string_attribute(group, "delayed_version");
        if (vstring == "1.0.0") {
            version.major = 1;
        } else {
            version = ritsuko::parse_version_string(vstring.c_str(), vstring.size(), /* skip_patch = */ true);
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
 * @return Details of the array after all delayed operations in `group` (and its children) have been applied.
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
 */
inline ArrayDetails validate(const std::string& path, const std::string& name, const Options& options) {
    H5::H5File handle(path, H5F_ACC_RDONLY);
    auto ghandle = handle.openGroup(name);
    return validate(ghandle, options);
}

/**
 * Overload of `validate()` that accepts a file path and group name, and uses the default `Options`. 
 * 
 * @param path Path to a HDF5 file.
 * @param name Name of the HDF5 group inside the file representing a delayed operation or array.
 *
 * @return Details of the array after all delayed operations have been applied.
 */
inline ArrayDetails validate(const std::string& path, const std::string& name) {
    H5::H5File handle(path, H5F_ACC_RDONLY);
    Options options;
    auto ghandle = handle.openGroup(name);
    return validate(ghandle, options);
}

}

#endif
