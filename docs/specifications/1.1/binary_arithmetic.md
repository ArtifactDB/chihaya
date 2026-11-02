

# Binary arithmetic (1.1)

## Overview

A binary arithmetic operation applies an element-wise arithmetic operation on two delayed objects of the same shape.

## Contents

A binary arithmetic operation is represented as a HDF5 group with the following attributes:

- `delayed_type` should be a scalar string dataset that contains `"operation"`.
- `delayed_operation` should be a scalar string dataset that contains `"binary arithmetic"`.

Inside the operation's group, we expect:

- A `left` group, containing a delayed object on the left of the arithmetic operation.
  The value type of this delayed object can be either boolean, integer or float.
  Booleans are promoted to integers for the purpose of this operation.
- A `right` group, containing a delayed object on the right of the operation.
  The value type of this delayed object can be either boolean, integer or float.
  Booleans are promoted to integers for the purpose of this operation.
  The `right` and `left` delayed objects should have exactly the same dimensions. 
- A `method` string scalar dataset, specifying the arithmetic method to use.
  This can be any one of `+`, `-`, `/`, `*`, `^`, `%%` (modulo) or `%/%` (integer division).

## Shape

The shape of the binary arithmetic operation's output object is equal to that of the `left` and `right` delayed objects.

## Type

The value type of the binary arithmetic operation's output object depends on the operation, the type of `left` and the type of `right`:

- For simple division, the output value type is always float.
- For integer division, the output value type is always integer.
- For all other operations:
  - If the value types of `left` and `right` are the same, the output of the arithmetic operation is equal to that type.
  - Otherwise, the less advanced type is promoted to a more advanced type, and the output of the arithmetic operation is equal to the promoted type.

No guarantees are provided with respect to the over/underflow of the output type.
