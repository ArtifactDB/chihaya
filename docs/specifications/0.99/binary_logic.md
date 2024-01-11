

# Binary logic (0.99)

## Overview

A binary logic operation involves an element-wise logical operation between two delayed objects of the same shape.

## Specification

A binary logic operation is represented as a HDF5 group with the following attributes:

- `delayed_type` should be a scalar string dataset, of any datatype that can be represented by a UTF-8 encoded string.
  It should contain the value `"operation"`.
- `delayed_operation` should be a scalar string dataset, of any datatype that can be represented by a UTF-8 encoded string.
  It should contain the value `"binary logic"`.

Inside the group, we expect:

- A `left` group, containing a delayed object on the left of the logic operation.
  The value type of this object can be either boolean, integer or float.
- A `right` group, containing a delayed object on the right of the operation.
  This should have exactly the same dimensions as the `left` object.
  The value type of this object can be either boolean, integer or float.
- A `method` string scalar dataset, specifying the logic method to use.
  This can be any one of `&&` or `||`.
  The datatype of the dataset should be representable by a UTF-8 encoded string.

The value type of the output object is always boolean.
For non-boolean `left` and `right`, values are converted to booleans with the usual rules (non-zero values are truthy, zero is falsey) before the logical operation.
