

# Unary comparison (0.99)

## Overview

A unary comparison involves applying an element-wise comparison on a delayed object with a scalar or vector as the other operand,
recycling the latter across dimensions as necessary.
This is "unary" in the sense that only one delayed object is involved.

## Specification

A unary comparison is represented as a HDF5 group with the following attributes:

- `delayed_type` should be a scalar string dataset, of any datatype that can be represented by a UTF-8 encoded string.
  It should contain the value `"operation"`.
- `delayed_operation` should be a scalar string dataset, of any datatype that can be represented by a UTF-8 encoded string.
  It should contain the value `"binary comparison"`.

Inside the group, we expect:

- A `seed` group, containing a delayed object for which comparison is to be applied.
  The value type of this object can be either boolean, integer, float, or string.
- A `method` string scalar dataset, specifying the comparison method to use.
  This can be any one of `==`, `<`, `>`, `>=`, `<=`, or `!=`. 
  The datatype of the dataset should be representable by a UTF-8 encoded string.
- A `side` string scalar dataset, describing the side of the `seed` object to apply the operation.
  This can be `"left"`, when `value` is applied to the left of `seed`, e.g., `value > seed`;
  or `"right"`, when `value` is applied to the right of `seed`, e.g., `seed > value`.
  The datatype of the dataset should be representable by a UTF-8 encoded string.
- A `value` dataset, representing the other (non-delayed object) operand.
  This may be either scalar or 1-dimensional.
  If `seed` contains strings, so should `value`; otherwise, both `seed` and `value` should be any of boolean, integer or float.
  The exact type representation is left to the implementation.

If `value` is 1-dimensional, we also expect:

- An `along` integer scalar dataset, specifying the dimension on which to apply the operation with `value`.
  The exact integer representation is left to the implementation.
  The length of `value` should be equal to the extent of the dimension specified in `along`.

`value` may contain a `missing_placeholder` attribute.
This should be a scalar dataset of the exact same datatype as `value`.
The value of this dataset is the missing placeholder used for all missing elements,
i.e., any elements in `value` with the same value as the placeholder should be treated as missing.

The value type of the output object is always boolean.
For `seed` and `value` that are of different types, the less advanced type is promoted to a more advanced type prior to performing the comparison.
