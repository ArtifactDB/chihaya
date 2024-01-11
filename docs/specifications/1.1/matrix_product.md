

# Matrix product (1.1)

## Overview

A matrix product involves performing a matrix multiplication between two 2-dimensional delayed objects.

## Specification

A delayed matrix product is represented as a HDF5 group with the following attributes:

- `delayed_type` should be a scalar string dataset, of any datatype that can be represented by a UTF-8 encoded string.
  It should contain the value `"operation"`.
- `delayed_operation` should be a scalar string dataset, of any datatype that can be represented by a UTF-8 encoded string.
  It should contain the value `"matrix product"`.

Inside the group, we expect:

- A `left_seed` group describing a delayed object, i.e., another delayed operation or array.
  This is used as the left hand side of the matrix product.
  It should have exactly two dimensions and its value type should be integer, boolean or float.
  (Booleans are promoted to integer for this operation.)
- A `left_orientation` scalar string dataset, specifying whether the `left_seed` array should be transposed (`"T"`) or not (`"N"`).
  The datatype of the dataset should be representable by a UTF-8 encoded string.
- A `right_seed` group describing a delayed object, i.e., another delayed operation or array.
  This is used as the right hand side of the matrix product.
  It should have exactly two dimensions and its value type should be integer, boolean or float.
  (Booleans are promoted to integer for this operation.)
- A `right_orientation` scalar string dataset, specifying whether the `right_seed` should be transposed (`"T"`) or not (`"N"`).
  The datatype of the dataset should be representable by a UTF-8 encoded string.

For example, setting `left orientation` to `"T"` and `right orientation` to `"N"` would be equivalent to `t(left_seed) * right_seed`.
This enables optimizations during the multiplication by avoiding the need to explicitly realize the transposition.

The extent of the common dimension should be the same between `left_seed` and `right_seed`.
If `left_orientation = "N"`, the common dimension of `left_seed` are the columns, otherwise it is the rows.
If `right_orientation = "N"`, the common dimension of `left_seed` are the rows, otherwise it is the columns.

If either `left_seed` or `right_seed` are floating-point, the output value type will also be float.
Otherwise, the output type will be integer.
