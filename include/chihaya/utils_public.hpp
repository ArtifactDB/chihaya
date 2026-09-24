#ifndef CHIHAYA_UTILS_PUBLIC_HPP
#define CHIHAYA_UTILS_PUBLIC_HPP

#include "H5Cpp.h"
#include "ritsuko/ritsuko.hpp"
#include "sanisizer/sanisizer.hpp"

#include <string>
#include <functional>
#include <vector>
#include <unordered_map>
#include <cstddef>

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

    ArrayDetails(ArrayType t, std::vector<std::size_t> d) : type(t), dimensions(std::move(d)) {}
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
    std::vector<std::size_t> dimensions;
};

/**
 * @cond
 */
struct Options;
/**
 * @endcond
 */

/**
 * Type of the registry of validation functions.
 * Each key is the name of a delayed array/operation and each value is a validation function.
 * See `validate()` for more details on the expected arguments and return type.
 */
typedef std::unordered_map<std::string, std::function<ArrayDetails(const H5::Group&, const ritsuko::Version&, const Options&)> > ValidateRegistry;

/**
 * @brief Options for `validate()`.
 */
struct Options {
    /**
     * Whether to skip extensive validation and just return the `ArrayDetails`.
     * If this is set to true, it is assumed that the array/operation is already valid.
     */
    bool details_only = false;

    /**
     * Size of the streaming chunks (in terms of the number of elements) to use for contiguous HDF5 datasets.
     * Ignored for chunked datasets where streaming chunk size is defined as the dataset's chunk size.
     */
    hsize_t contiguous_chunk_size = sanisizer::cap<hsize_t>(10000);

    /**
     * Custom registry of functions to be used by `validate()` on arrays.
     * If a custom function is provided for an existing array type in **chihaya**, it is used instead of the default function.
     */
    ValidateRegistry array_validate_registry;

    /**
     * Custom registry of functions to be used by `validate()` on operations.
     * If a custom function is provided for an existing operation type in **chihaya**, it is used instead of the default function.
     */
    ValidateRegistry operation_validate_registry;
};

}

#endif
