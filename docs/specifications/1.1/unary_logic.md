

# Unary logic (1.1)

## Overview

A unary logic operation involves applying an element-wise logic operation on a delayed object with a scalar or vector as the other operand,
recycling the latter across dimensions as necessary.
This is "unary" in the sense that only one delayed object is involved.

## Specification

A unary logic operation is represented as a HDF5 group with the following attributes:

- `delayed_type` should be a scalar string dataset, of any datatype that can be represented by a UTF-8 encoded string.
  It should contain the value `"operation"`.
- `delayed_operation` should be a scalar string dataset, of any datatype that can be represented by a UTF-8 encoded string.
  It should contain the value `"binary logic"`.

Inside the group, we expect:

- A `seed` group, containing a delayed object for which logic is to be applied.
  The value type of this object can be either boolean, integer or float.
- A `method` string scalar dataset, specifying the logic method to use.
  This can be any one of `!`, `&&`, `||` .
  The datatype of the dataset should be representable by a UTF-8 encoded string.

For `method != "!"`, we also expect:

- A `side` dataset, describing the side of the `seed` object to apply the operation.
  This can be `"left"`, when `value` is applied to the left of `seed`, e.g., `value > seed`;
  or `"right"`, when `value` is applied to the right of `seed`, e.g., `seed > value`.
  The datatype of the dataset should be representable by a UTF-8 encoded string.
- A `value` dataset, representing the other (non-delayed object) operand.
  This may be either scalar or 1-dimensional.
  The dataset should have a `type` scalar string attribute that specifies the value type of the other operand. 
  The attribute should be of a datatype that can be represented by a UTF-8 encoded string, and should hold one of the following values:
  - `"INTEGER"`, in which case `data` should have a datatype that fits into a 32-bit signed integer.
  - `"FLOAT"`, in which case `data` should have a datatype that fits into a 64-bit float.
  - `"BOOLEAN"`, in which case `data` should have a datatype that fits into a 8-bit signed integer.

If `value` is 1-dimensional, we also expect:

- An `along` scalar integer dataset, specifying the dimension on which to apply the operation with `value`.
  This dataset should have a datatype that fits into a 64-bit unsigned integer.
  The length of `value` should be equal to the extent of the dimension specified in `along`.

`value` may contain a `missing_placeholder` attribute.
This should be a scalar dataset of the exact same datatype as `value`.
The value of this dataset is the missing placeholder used for all missing elements,
i.e., any elements in `value` with the same value as the placeholder should be treated as missing.
Check out the [HDF5 policy draft (v0.1.0)](https://github.com/ArtifactDB/Bioc-HDF5-policy/tree/v0.1.0) for more details.

The value type of the output object is always boolean.
For non-boolean `seed` and `value`, values are converted to booleans with the usual rules (non-zero values are truthy, zero is falsey) before the logical operation.
