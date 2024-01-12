

# Constant array (1.0)

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
  Any integer datatype can be used here.
  All values should be non-negative.
- A `value` scalar dataset, containing the value of the constant array.
  This can be either boolean, integer, float or string; the exact type is left to the implementation.

`value` may contain a `missing_placeholder` attribute.
This should be a scalar dataset of the same datatype class as `value`, specifying the placeholder value used for all missing elements,
i.e., any elements in `value` with the same value as the placeholder should be treated as missing.
(Note that, for floating-point datasets, the placeholder itself may be NaN, so byte-wise comparison should be used when checking for missingness.)

The value type of the constant array is inferred from the datatype of `value`: integer, float or string.