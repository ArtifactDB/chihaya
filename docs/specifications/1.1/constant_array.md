

# Constant array (1.1)

## Overview

A constant array is an array with a constant value, typically used to store placeholder values for missingness.

## Specification

A constant array is represented as a HDF5 group with the following attributes:

- `delayed_type` should be a scalar string dataset, of any datatype that can be represented by a UTF-8 encoded string.
  It should contain the value `"array"`.
- `delayed_array` should be a scalar string dataset, of any datatype that can be represented by a UTF-8 encoded string.
  It should contain the value `"constant array"`.

Inside the group, we expect:

- A `dimensions` dataset, specifying the dimensions of the constant array.
  This should be a 1-dimensional dataset of non-zero length equal to the number of dimensions.
  It can be of any datatype that can be represented by a 64-bit unsigned integer.
- A `value` scalar dataset, containing the value of the constant array.
  This should have a `type` scalar string attribute that specifies the value type of the array.
  The attribute should be of a datatype that can be represented by a UTF-8 encoded string, and should hold one of the following values:
  - `"INTEGER"`, in which case `data` should have a datatype that fits into a 32-bit signed integer.
  - `"FLOAT"`, in which case `data` should have a datatype that fits into a 64-bit float.
  - `"BOOLEAN"`, in which case `data` should have a datatype that fits into a 8-bit signed integer.
  - `"STRING"`, in which case `data` should have a datatype that can be represented by a UTF-8 encoded string.

`value` may contain a `missing_placeholder` attribute.
This should be a scalar dataset of the exact same datatype as `value` (except for a string ` value `, in which case only the same datatype class is required).
The value of this dataset is the missing placeholder used for all missing elements,
i.e., any elements in `value` with the same value as the placeholder should be treated as missing.
Check out the [HDF5 policy draft (v0.1.0)](https://github.com/ArtifactDB/Bioc-HDF5-policy/tree/v0.1.0) for more details.

The value type of the constant array is set to the `type` attribute of `value`.
