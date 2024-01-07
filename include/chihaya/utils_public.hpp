#ifndef CHIHAYA_UTILS_PUBLIC_HPP
#define CHIHAYA_UTILS_PUBLIC_HPP

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

}

#endif
