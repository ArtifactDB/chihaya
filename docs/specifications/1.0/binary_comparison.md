

# Binary comparison (1.0)

## Overview

A binary comparison operation performs an element-wise comparison between two delayed objects of the same shape.

## Contents

A binary comparison operation is represented as a HDF5 group with the following attributes:

- `delayed_type` should be a scalar string dataset that contains `"operation"`.
- `delayed_operation` should be a scalar string dataset that contains `"binary comparison"`.

Inside the operation's group, we expect:

- A `left` group, containing a delayed object on the left of the comparison operation.
  If `right` contains strings, so should `left`; otherwise, the value type of `left` can any of boolean, integer or float.
- A `right` group, containing a delayed object on the right of the operation.
  This should have exactly the same dimensions as the `left` delayed object.
  If `left` contains strings, so should `right`; otherwise, the value type of `right` can be any of boolean, integer or float.
- A `method` string scalar dataset, specifying the comparison method to use.
  This can be any one of `==`, `<`, `>`, `>=`, `<=`, or `!=`. 

## Shape

The shape of the binary comparison operation's output object is equal to that of the `left` and `right` delayed objects.

## Type

The value type of the binary comparison operation's output object is always boolean.
For non-string `left` and `right` that are of different value types, the less advanced type is promoted to a more advanced type prior to comparison.
