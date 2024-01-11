

# Custom array (1.0)

## Overview

A custom array involves some application-specific custom array type that provides its dimensions and type but no further details.
This allows third-party developers to extend the chihaya specification with their own array definitions.
The custom array is most commonly used to embed references to external resources, e.g., arrays stored in custom databases,
thus avoiding a redundant copy of a large arrays when we only want to preserve the delayed operations.
Of course, it is assumed that clients will know how to retrieve specific resources from the remotes.

## Specification

Each custom array is represented as a HDF5 group with the following attributes:

- `delayed_type` should be a scalar string dataset, of any datatype that can be represented by a UTF-8 encoded string.
  It should contain the value `"array"`.
- `delayed_array` should be a scalar string dataset, of any datatype that can be represented by a UTF-8 encoded string.
  The value of the string should start with `"custom "` (note the space).
  Implementations are expected to add their own descriptors to define specific interprations.

Inside the group, we expect at least the following children:

- `dimensions`, specifying the dimensions of the external array.
  This should be a 1-dimensional dataset of non-zero length equal to the number of dimensions.
  Any integer datatype can be used here.
  All values should be non-negative.
- `type`, a scalar string dataset specifying the value type of the array.
  This should be one of `"BOOLEAN"`, `"INTEGER"`, `"FLOAT"` or `"STRING"`.
  The datatype of the dataset should be representable by a UTF-8 encoded string.
