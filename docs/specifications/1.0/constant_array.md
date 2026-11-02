

# Constant array (1.0)

## Overview

A constant array is an array with a constant value, typically used to store placeholder values for missingness.

## Contents

A constant array is represented as a HDF5 group with the following attributes:

- `delayed_type` should be a scalar string dataset that contains `"array"`.
- `delayed_array` should be a scalar string dataset that contains `"constant array"`.

Inside the group, we expect:

- A `dimensions` dataset, specifying the dimensions of the constant array.
  This should be a 1-dimensional dataset of non-zero length equal to the number of dimensions.
  Any integer datatype can be used here.
  All values should be non-negative.
- A `value` scalar dataset, containing the value of the constant array.
  Its datatype can be either integer, float or string; the exact type is left to the implementation.
  Optionally, integer datatypes may have a `is_boolean` scalar integer attribute, where a truthy value indicates that `value` should be interpreted as a boolean.

`value` may contain a `missing_placeholder` attribute.
This should be a scalar attribute of the same datatype class as ` value `, specifying the placeholder value used for all missing elements,
i.e., any elements in ` value ` with the same value as the placeholder should be treated as missing.

## Shape

The shape of the constant array is defined as the contents of the `dimensions` dataset.

## Type

The value type of the constant array is inferred from the datatype of `value`: integer, boolean, float or string.
