

# Subset (0.99)

## Overview

A delayed subset involves applying a slice to zero, one or more dimensions of a delayed object.
If slices are applied to multiple dimensions, the subset is defined as the outer product of all slices.

## Specification

A delayed subset is represented as a HDF5 group with the following attributes:

- `delayed_type` should be a scalar string dataset, of any datatype that can be represented by a UTF-8 encoded string.
  It should contain the value `"operation"`.
- `delayed_operation` should be a scalar string dataset, of any datatype that can be represented by a UTF-8 encoded string.
  It should contain the value `"subset"`.

Inside the group, we expect:

- A `seed` group, containing the delayed object to be subsetted. 
  This can be of any value type - boolean, integer, float or string.
- An `index` group, representing a [list](_general.md#lists) of length equal to the number of dimensions in the `seed`.
  Each child entry is named after a dimension of `seed` and contains the indices of interest along that dimension.
  Each entry should be a 1-dimensional integer dataset containing 0-based indices that are less than the extent of its dimension.
  The exact integer representation is left to the implementation.
  The absence of an entry for a dimension indicates that no subsetting is to be performed, i.e., the full extent of that dimension is present in the result object.

The value type of the output object is the same as that of the `seed`; only the dimensions are changed.
