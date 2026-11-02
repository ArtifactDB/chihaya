

# Unary arithmetic (1.1)

## Overview

A unary arithmetic operation applies an element-wise arithmetic operation on a delayed object with a scalar or vector as the other operand,
recycling the latter across dimensions as necessary.
This is "unary" in the sense that only one delayed object is involved.

## Contents 

A unary arithmetic operation is represented as a HDF5 group with the following attributes:

- `delayed_type` should be a scalar string dataset that contains `"operation"`.
- `delayed_operation` should be a scalar string dataset that contains `"unary arithmetic"`.

Inside the operation's group, we expect:

- A `seed` group, containing a delayed object for which arithmetic is to be applied.
  The value type of this object should be either boolean, integer or float.
  Booleans are promoted to integers for the purpose of this operation.
- A `method` string scalar dataset, specifying the arithmetic method to use.
  This can be any one of `+`, `-`, `/`, `*`, `^`, `%%` (modulo) or `%/%` (integer division).
- A `side` string scalar dataset, describing the side of the `seed` delayed object to apply the operation.
  This can be `"left"`, when `value` is applied to the left of `seed`, e.g., `value - seed`;
  or `"right"`, when `value` is applied to the right of `seed`, e.g., `seed - value`.
  For `+` and `-` as pure unary methods, this may also be `"none"`.

For `side != "none"`, the operation's group should also contain:

- A `value` dataset, representing the other (non-delayed object) operand.
  This may be either scalar or 1-dimensional.
  The dataset should have a `type` scalar string attribute that specifies the value type of the other operand. 
  The attribute should contain one of the following values:
  - `"INTEGER"`, in which case `data` should have a datatype that fits into a 32-bit signed integer.
  - `"FLOAT"`, in which case `data` should have a datatype that fits into a 64-bit float.
  - `"BOOLEAN"`, in which case `data` should have a datatype that fits into a 8-bit signed integer.
    Booleans are promoted to integers for the purpose of this operation.

If `side != "none"` and `value` is 1-dimensional, the operation's group should also contain:

- An `along` scalar integer dataset, specifying the dimension on which to apply the operation with `value`.
  This dataset should have a datatype that fits into a 64-bit unsigned integer.
  The length of `value` should be equal to the extent of the dimension specified in `along`.

`value` may contain a `missing_placeholder` attribute.
This should be a scalar attribute of the exact same datatype as `value`. 
The value of this attribute is the missing placeholder used for all missing elements,
i.e., any elements in ` value ` with the same value as the placeholder should be treated as missing.

If `side != "none"` and `value` is scalar, the arithmetic operation is performed on each element of the `seed` delayed object and the scalar `value`, regardless of the former's position.
If `side != "none"` and `value` is 1-dimensional, the value of `value` to be used in the arithmetic operation depends on the position of the element in the `seed` delayed object.
Given an element of `seed` at the array coordinates $(s_0, s_1, ..., s_D)$ for dimensionality $D$,
the $s_j$-th element of `value` will be used in the arithmetic operation where $j$ is the value in the `along` dataset.

## Shape 

The shape of the unary arithmetic operation's output object is the same as that of the `seed` delayed object.

## Type

The value type of the unary arithmetic operation's output object depends on the operation, the value type of `seed` and the value type of `value`:

- For `/`, the output value type is always float.
- For `%/%`, the output value type is always integer.
- For all other operations:
  - If the types of `seed` and `value` are the same, the output type of the arithmetic operation is equal to that type.
  - Otherwise, the less advanced type is promoted to a more advanced type, and the output type of the arithmetic operation is equal to the promoted type.

Note that no guarantees are provided with respect to the over/underflow of the output type.
