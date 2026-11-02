

# Unary comparison (0.99)

## Overview

A unary comparison operation applies an element-wise comparison on a delayed object with a scalar or vector as the other operand,
recycling the latter across dimensions as necessary.
This is "unary" in the sense that only one delayed object is involved.

## Contents

A unary comparison operation is represented as a HDF5 group with the following attributes:

- `delayed_type` should be a scalar string dataset that contains `"operation"`.
- `delayed_operation` should be a scalar string dataset that contains `"binary comparison"`.

Inside the operation's group, we expect:

- A `seed` group, containing a delayed object for which comparison is to be applied.
  The value type of this object can be either boolean, integer, float, or string.
- A `method` string scalar dataset, specifying the comparison method to use.
  This can be any one of `==`, `<`, `>`, `>=`, `<=`, or `!=`. 
- A `side` string scalar dataset, describing the side of the `seed` object to apply the operation.
  This can be `"left"`, when `value` is applied to the left of `seed`, e.g., `value > seed`;
  or `"right"`, when `value` is applied to the right of `seed`, e.g., `seed > value`.
- A `value` dataset, representing the other (non-delayed object) operand.
  This may be either scalar or 1-dimensional.
  If `seed` contains strings, so should `value`; otherwise, both `seed` and `value` should be any of boolean, integer or float.
  The exact type representation is left to the implementation.

If `value` is 1-dimensional, the operation's group should also contain:

- An `along` integer scalar dataset, specifying the dimension on which to apply the operation with `value`.
  The exact integer representation is left to the implementation.
  The length of `value` should be equal to the extent of the dimension specified in `along`.

`value` may contain a `missing_placeholder` attribute.
This should be a scalar attribute of the exact same datatype as `value`. 
The value of this attribute is the missing placeholder used for all missing elements,
i.e., any elements in ` value ` with the same value as the placeholder should be treated as missing.

If `value` is scalar, the comparison is performed on each element of the `seed` delayed object and the scalar `value`, regardless of the former's position.
If `value` is 1-dimensional, the value of `value` to be used in the comparison depends on the position of the element in the `seed` delayed object.
Given a element of `seed` at the array coordinates $(s_0, s_1, ..., s_D)$ for dimensionality $D$,
the $s_j$-th element of `value` will be used in the comparison where $j$ is the value in the `along` dataset.

## Shape

The shape of the unary comparison operation's output object is the same as that of the `seed` delayed object.

## Type

The value type of the unary comparion operation's output object is always boolean.
For `seed` and `value` that are of different types, the less advanced type is promoted to a more advanced type prior to performing the comparison.
