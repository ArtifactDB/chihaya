

# Unary logic (0.99)

## Overview

A unary logic operation applies an element-wise logic operation on a delayed object with a scalar or vector as the other operand,
recycling the latter across dimensions as necessary.
This is "unary" in the sense that only one delayed object is involved.

## Contents

A unary logic operation is represented as a HDF5 group with the following attributes:

- `delayed_type` should be a scalar string dataset that contains `"operation"`.
- `delayed_operation` should be a scalar string dataset that contains `"binary logic"`.

Inside the group, we expect:

- A `seed` group, containing a delayed object for which logic is to be applied.
  The value type of this object can be either boolean, integer or float.
- A `method` string scalar dataset, specifying the logic method to use.
  This can be any one of `!`, `&&`, `||` .

For `method != "!"`, we also expect:

- A `side` dataset, describing the side of the `seed` object to apply the operation.
  This can be `"left"`, when `value` is applied to the left of `seed`, e.g., `value > seed`;
  or `"right"`, when `value` is applied to the right of `seed`, e.g., `seed > value`.
- A `value` dataset, representing the other (non-delayed object) operand.
  This may be either scalar or 1-dimensional.
  This can be either integer or float; the exact type is left to the implementation.

If `value` is 1-dimensional, we also expect:

- An `along` scalar integer dataset, specifying the dimension on which to apply the operation with `value`.
  The exact integer representation is left to the implementation.
  The length of `value` should be equal to the extent of the dimension specified in `along`.

`value` may contain a `missing_placeholder` attribute.
This should be a scalar attribute of the exact same datatype as `value`. 
The value of this attribute is the missing placeholder used for all missing elements,
i.e., any elements in ` value ` with the same value as the placeholder should be treated as missing.

If `value` is scalar, the logic operation is performed on each element of the `seed` delayed object and the scalar `value`, regardless of the former's position.
If `value` is 1-dimensional, the value of `value` to be used in the logic operation depends on the position of the element in the `seed` delayed object.
Given a element of `seed` at the array coordinates $(s_0, s_1, ..., s_D)$ for dimensionality $D$,
the $s_j$-th element of `value` will be used in the logic operation where $j$ is the value in the `along` dataset.

## Shape 

The shape of the unary logic operation's output object is the same as that of the `seed` delayed object.

## Type

The value type of the unary logic operation's output object is always boolean.
For non-boolean `seed` and `value`, values are converted to booleans with the usual rules (non-zero values are truthy, zero is falsey) before the logical operation.
