#ifndef CHIHAYA_UTILS_PUBLIC_HPP
#define CHIHAYA_UTILS_PUBLIC_HPP

#include "H5Cpp.h"
#include "ritsuko/ritsuko.hpp"

#include <string>
#include <functional>
#include <vector>

/**
 * @file utils_public.hpp
 *
 * @brief Various public utilities.
 */

namespace chihaya {

/**
 * Type of the array.
 * Operations involving mixed types will generally result in promotion to the more advanced types,
 * e.g., an `INTEGER` and `FLOAT` addition will result in promotion to `FLOAT`.
 * Note that operations involving the same types are not guaranteed to preserve type,
 * e.g., `INTEGER` division is assumed to produce a `FLOAT`.
 */
enum ArrayType { BOOLEAN = 0, INTEGER = 1, FLOAT = 2, STRING = 3 }; // giving explicit values for comparisons to work.

/**
 * @brief Details about an array.
 *
 * This contains the type and dimensionality of the array.
 * The exact type representation of the array is left to the implementation;
 * we do not make any guarantees about precision, width or signedness.
 */
struct ArrayDetails {
    /**
     * @cond
     */
    ArrayDetails() {}

    ArrayDetails(ArrayType t, std::vector<size_t> d) : type(t), dimensions(std::move(d)) {}
    /**
     * @endcond
     */

    /**
     * Type of the array.
     */
    ArrayType type;

    /** 
     * Dimensions of the array.
     * Values should be non-negative.
     */
    std::vector<size_t> dimensions;
};

/**
 * @brief Callbacks to invoke on each delayed object.
 *
 * Advanced users can specify callbacks that are called on each call to `validate()`.
 * This is typically used to perform extra application-specific validation steps,
 * or to collect statistics like the "depth" of the operation tree.
 */
struct Callbacks {
    /**
     * Callback to be applied on each array.
     * This accepts the name of the array type, a handle to the array's HDF5 group, and the version.
     * If null, this is not called.
     */
    std::function<void(const std::string&, const H5::Group&, const ritsuko::Version&)> array;

    /**
     * Callback to be applied on each operation.
     * This accepts the name of the operation type, a handle to the operation's HDF5 group, and the version.
     * If null, this is not called.
     */
    std::function<void(const std::string&, const H5::Group&, const ritsuko::Version&)> operation;
};

}

#endif
