

# Custom array (1.1)

## Overview

A custom array involves some application-specific custom array type that provides its dimensions and type but no further details.
This allows third-party developers to extend the chihaya specification with their own array definitions.
The custom array is most commonly used to embed references to external resources, e.g., arrays stored in custom databases,
thus avoiding a redundant copy of a large arrays when we only want to preserve the delayed operations.
Of course, it is assumed that clients will know how to retrieve specific resources from the remotes.

## Contents

Each custom array is represented as a HDF5 group with the following attributes:

- `delayed_type` should be a scalar string dataset that contains `"array"`.
- `delayed_array` should be a scalar string dataset that contains a string starting with `"custom "` (note the space).
  Implementations are expected to add their own descriptors to define specific interprations.

Inside the array's group, we expect at least the following children:

- `dimensions`, specifying the dimensions of the external array.
  This should be a 1-dimensional dataset of non-zero length equal to the number of dimensions.
  It can be of any datatype that can be represented by a 64-bit unsigned integer.
- `type`, a scalar string dataset specifying the value type of the array.
  This should be one of `"BOOLEAN"`, `"INTEGER"`, `"FLOAT"` or `"STRING"`.

## Shape

The shape of the custom array is defined as the contents of the `dimensions` dataset.

## Type

The value type of the custom array is defined by the `type` dataset.

