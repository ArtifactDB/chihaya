

# Combine (0.99)

## Overview

A combine operation combines one or more delayed objects along a single dimension.

## Contents 

A combine operation is represented as a HDF5 group with the following attributes:

- `delayed_type` should be a scalar string dataset that contains `"operation"`.
- `delayed_operation` should be a scalar string dataset that contains `"combine"`.

Inside the operation's group, we expect:

- A `seeds` group.
  This should be a [list](_general.md#lists) of length equal to the number of delayed objects to be combined.
  Each list element is a HDF5 group that represents a delayed object.
  That is, the $i$-th delayed object should be represented by the `seeds/{i}` HDF5 group for zero-based $i$, e.g., `seeds/0` for the first delayed object.
  All delayed objects should have the same dimensionality.
  They should also have the same dimension extents, with the exception of the dimension specified in `along`.
  No list elements should be missing within `seeds`.
- An `along` scalar dataset 
  of any integer datatype.
  This is a 0-based index that specifies the dimensions on which to combine delayed objects in `seeds`.
  It should be a non-negative value that is less than the dimensionality of each delayed object.

For readers, delayed objects are combined in the same order as they are stored in the `seeds` group.

## Shape

The shape of the combine operation's output object is equal to the dimension extent of the first delayed object stored in the `seeds` group -
except for the dimension specified in `along`, where the extent is equal to the sum of extents for that dimension across all delayed objects in `seeds`. 

If `seeds` is empty, the output object is assumed to have zero dimensionality.

## Type

If all delayed objects in `seeds` have the same value type, the combine operation's output object will also be of that type.
Otherwise, the type of the output object is set to the most advanced value type among all `seeds` objects.
For example, a mixture of integer and float objects will result in a float output, where all objects are promoted to float before combining them.
Note that mixing of string and non-string objects is not allowed.

If `seeds` is empty, the output object has boolean type.
