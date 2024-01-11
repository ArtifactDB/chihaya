

# Binary comparison (1.1)

## Overview

A binary comparison operation involves an element-wise comparison between two delayed objects of the same shape.

## Specification

A binary comparison operation is represented as a HDF5 group with the following attributes:

- `delayed_type` should be a scalar string dataset, of any datatype that can be represented by a UTF-8 encoded string.
  It should contain the value `"operation"`.
- `delayed_operation` should be a scalar string dataset, of any datatype that can be represented by a UTF-8 encoded string.
  It should contain the value `"binary comparison"`.

Inside the group, we expect:

- A `left` group, containing a delayed object on the left of the comparison operation.
  If `right` contains strings, so should `left`; otherwise, the value type of `left` can any of boolean, integer or float.
- A `right` group, containing a delayed object on the right of the operation.
  This should have exactly the same dimensions as the `left` object.
  If `left` contains strings, so should `right`; otherwise, the value type of `right` can be any of boolean, integer or float.
- A `method` string scalar dataset, specifying the comparison method to use.
  This can be any one of `==`, `<`, `>`, `>=`, `<=`, or `!=`. 
  The datatype of the dataset should be representable by a UTF-8 encoded string.

The value type of the output object is always boolean.
For non-string `left` and `right` that are of different value types, the less advanced type is promoted to a more advanced type prior to comparison.
