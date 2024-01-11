

# Unary arithmetic (0.99)

## Overview

A unary arithmetic operation involves applying an element-wise arithmetic operation on a delayed object with a scalar or vector as the other operand,
recycling the latter across dimensions as necessary.
This is "unary" in the sense that only one delayed object is involved.

## Specification

A unary arithmetic operation is represented as a HDF5 group with the following attributes:

- `delayed_type` should be a scalar string dataset, of any datatype that can be represented by a UTF-8 encoded string.
  It should contain the value `"operation"`.
- `delayed_operation` should be a scalar string dataset, of any datatype that can be represented by a UTF-8 encoded string.
  It should contain the value `"unary arithmetic"`.

Inside the group, we expect:

- A `seed` group, containing a delayed object for which arithmetic is to be applied.
  The value type of this object should be either boolean, integer or float.
  Booleans are promoted to integers for the purpose of this operation.
- A `method` string scalar dataset, specifying the arithmetic method to use.
  This can be any one of `+`, `-`, `/`, `*`, `^`, `%%` (modulo) or `%/%` (integer division).
  The datatype of the dataset should be representable by a UTF-8 encoded string.
- A `side` string scalar dataset, describing the side of the `seed` object to apply the operation.
  This can be `"left"`, when `value` is applied to the left of `seed`, e.g., `value - seed`;
  or `"right"`, when `value` is applied to the right of `seed`, e.g., `seed - value`.
  For `+` and `-` as pure unary methods, this may also be `"none"`.
  The datatype of the dataset should be representable by a UTF-8 encoded string.

For `side != "none"`, we also expect:

- A `value` dataset, representing the other (non-delayed object) operand.
  This may be either scalar or 1-dimensional.
  This can be either integer or float; the exact type is left to the implementation.

If `value` is 1-dimensional, we also expect:

- An `along` scalar integer dataset, specifying the dimension on which to apply the operation with `value`.
  The exact integer representation is left to the implementation.
  The length of `value` should be equal to the extent of the dimension specified in `along`.

`value` may contain a `missing_placeholder` attribute.
This should be a scalar dataset of the exact same datatype as `value`.
The value of this dataset is the missing placeholder used for all missing elements,
i.e., any elements in `value` with the same value as the placeholder should be treated as missing.

The value type of the output object depends on the operation, the value type of `seed` and the value type of `value`:

- For simple division, the output value type is always float.
- For integer division, the output value type is always integer.
- For all other operations:
  - If the types of `seed` and `value` are the same, the output type of the arithmetic operation is equal to that type.
  - Otherwise, the less advanced type is promoted to a more advanced type, and the output type of the arithmetic operation is equal to the promoted type.

Note that no guarantees are provided with respect to the over/underflow of the output type.
