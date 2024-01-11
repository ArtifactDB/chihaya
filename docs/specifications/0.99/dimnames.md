

# Dimnames assignment (0.99)

## Overview 

A dimnames assignment operation involves changing the names along one or more of the dimensions of the array.

## Specification

A delayed subsetting operation is represented as a HDF5 group with the following attributes:

- `delayed_type` should be a scalar string dataset, of any datatype that can be represented by a UTF-8 encoded string.
  It should contain the value `"operation"`.
- `delayed_operation` should be a scalar string dataset, of any datatype that can be represented by a UTF-8 encoded string.
  It should contain the value `"dimnames"`.

Inside the group, we expect:

- A `seed` group, containing a delayed object for which dimnames are to be assigned.
  This can be of any value type - boolean, integer, float or string.
- A `dimnames` group, representing a [list](_general.md#lists) of length equal to the number of dimensions in the `seed`.
  Each child entry corresponds to a dimension of `seed` and contains the names along that dimension.
  Each entry should be a 1-dimensional string dataset of length equal to the extent of its dimension.
  The datatype of each dataset should be representable by a UTF-8 encoded string.
  If a particular dataset is absent, no names are attached to the corresponding dimension.
  It is assumed that each string in each dataset is not missing.

The value type of the output array is equal to the value type of the `seed`.
