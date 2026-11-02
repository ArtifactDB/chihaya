

# Constant array (1.1)

## Overview

A constant array is an array with a constant value, typically used to store placeholder values for missingness.

## Contents

A constant array is represented as a HDF5 group with the following attributes:

- `delayed_type` should be a scalar string dataset that contains `"array"`.
- `delayed_array` should be a scalar string dataset that contains `"constant array"`.

Inside the group, we expect:

- A `dimensions` dataset, specifying the dimensions of the constant array.
  This should be a 1-dimensional dataset of non-zero length equal to the number of dimensions.
  It can be of any datatype that can be represented by a 64-bit unsigned integer.
- A `value` scalar dataset, containing the value of the constant array.
  This should have a `type` scalar string attribute that specifies the value type of the array.
  The attribute should contain one of the following values:

  - `"INTEGER"`, in which case `data` should have a datatype that fits into a 32-bit signed integer.
  - `"FLOAT"`, in which case `data` should have a datatype that fits into a 64-bit float.
  - `"BOOLEAN"`, in which case `data` should have a datatype that fits into a 8-bit signed integer.
  - `"STRING"`, in which case `data` should have a string datatype.

`value` may contain a `missing_placeholder` attribute.
This should be a scalar attribute of the exact same datatype as `value` (except for a string ` value `, in which case only any string datatype may be used). 
The value of this attribute is the missing placeholder used for all missing elements,
i.e., any elements in ` value ` with the same value as the placeholder should be treated as missing.

## Shape

The shape of the constant array is defined as the contents of the `dimensions` dataset.

## Type

The value type of the constant array is set to the `type` attribute of `value`.
