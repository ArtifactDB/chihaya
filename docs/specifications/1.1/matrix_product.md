

# Matrix product (1.1)

## Overview

A matrix product operation performs a matrix multiplication between two 2-dimensional delayed objects.

## Contents 

A matrix product operation is represented as a HDF5 group with the following attributes:

- `delayed_type` should be a scalar string dataset that contains `"operation"`.
- `delayed_operation` should be a scalar string dataset that contains `"matrix product"`.

Inside the operation's group, we expect:

- A `left_seed` group representing a delayed object to be used as the left hand side of the matrix product.
  It should have exactly two dimensions and its value type should be integer, boolean or float.
  (Booleans are promoted to integer for this operation.)
- A `left_orientation` scalar string dataset, specifying whether the `left_seed` delayed object should be transposed (`"T"`) or not (`"N"`) when computing the matrix product.
- A `right_seed` group representing a delayed object to be used as the right hand side of the matrix product.
  It should have exactly two dimensions and its value type should be integer, boolean or float.
  (Booleans are promoted to integer for this operation.)
- A `right_orientation` scalar string dataset, specifying whether the `right_seed` delayed object should be transposed (`"T"`) or not (`"N"`) when computing the matrix product.

For example, setting `left orientation` to `"T"` and `right orientation` to `"N"` would be equivalent to `t(left_seed) * right_seed`.
This enables optimizations during the multiplication by avoiding the need to explicitly realize the transposition.

The extent of the common dimension should be the same between the `left_seed` and `right_seed` delayed objects.
If `left_orientation = "N"`, the common dimension of the `left_seed` delayed object consists of its columns, otherwise rows.
If `right_orientation = "N"`, the common dimension of the `right_seed` delayed object consists of its rows, otherwise columns.

## Shape

The matrix product operation's output array is 2-dimensional.

The number of rows of the output array is equal to the number of rows of the `left_seed` delayed object if `left_orientation = "N"`.
Otherwise, it is equal to the number of columns of the `left_seed` delayed object.

The number of columns of the output array is equal to the number of columns of the `right_seed` delayed object if `right_orientation = "N"`.
Otherwise, it is equal to the number of rows of the `right_seed` delayed object.

## Type

If either `left_seed` or `right_seed` are floating-point, the value type of the matrix product operation's output array will also be float.
Otherwise, the value type of the output array will be integer.
