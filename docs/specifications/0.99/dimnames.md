

# Dimnames assignment (0.99)

## Overview 

A dimnames assignment operation involves changing the names along one or more of the dimensions of the array.

## Contents

A dimnames assignment operation is represented as a HDF5 group with the following attributes:

- `delayed_type` should be a scalar string dataset that contains `"operation"`.
- `delayed_operation` should be a scalar string dataset that contains `"dimnames"`.

Inside the operation's group, we expect:

- A `seed` group, containing a delayed object to which dimnames are to be assigned.
  This can be of any value type - boolean, integer, float or string.
- A `dimnames` group, representing a [list](_general.md#lists) of length equal to the number of dimensions in the `seed` delayed object.
  If a child object exists at `dimnames/{i}`, it should be a 1-dimensional string dataset of length equal to the extent of the $i$-th dimension of `seed`, 
  e.g., `dimnames/0` contains the names for the first dimension.
  Each entry of `dimnames/{i}` contains a name of the corresponding element of the $i$-th dimension of `seed`.
  If no child object is present at `dimnames/{i}`, the $i$-th dimension is unnamed.

## Shape

The shape of the dimnames assignment operation's output array is equal to that of the `seed`.

## Type

The value type of the dimnames assignment operation's output array is equal to that of the `seed`.
