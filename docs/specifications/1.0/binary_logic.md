

# Binary logic (1.0)

## Overview

A binary logic operation involves an element-wise logical operation between two delayed objects of the same shape.

## Contents 

A binary logic operation is represented as a HDF5 group with the following attributes:

- `delayed_type` should be a scalar string dataset that contains `"operation"`.
- `delayed_operation` should be a scalar string dataset that contains `"binary logic"`.

Inside the operation's group, we expect:

- A `left` group, containing a delayed object on the left of the logic operation.
  The value type of this object can be either boolean, integer or float.
- A `right` group, containing a delayed object on the right of the operation.
  This should have exactly the same dimensions as the `left` delayed object.
  The value type of this object can be either boolean, integer or float.
- A `method` string scalar dataset, specifying the logic method to use.
  This can be any one of `&&` or `||`.

## Shape

The shape of the binary logic operation's output object is equal to that of the `left` and `right` delayed objects.

## Type

The value type of the binary logic operation's output object is always boolean.
For non-boolean `left` and `right`, values are converted to booleans with the usual rules (non-zero values are truthy, zero is falsey) before the logical operation.
