

# Unary logic (1.0)

## Overview

A unary math operation applies an element-wise mathematical operation to a delayed object.

## Contents

A unary math operation is represented as a HDF5 group with the following attributes:

- `delayed_type` should be a scalar string dataset that contains `"operation"`.
- `delayed_operation` should be a scalar string dataset that contains `"unary math"`.

Inside the operation's group, we expect:

- A `seed` group, containing a delayed object for which the mathematical operation is to be applied.
  The value type of this object can be boolean (coerced to integer), integer or float.
- A `method` string scalar dataset, specifying the mathematical operation to perform.
  The value of the string can be any one of:
  - `abs`, absolute value.
  - `log1p`, log-transformation after adding 1.
  - `sqrt`, square root.
  - `exp`, exponential.
  - `expm1`, exponential followed by subtraction of 1.
  - `sign`, sign of the value (-1, 0, or 1).
  - `ceiling`, ceiling.
  - `floor`, floor.
  - `trunc`, truncate to zero.
  - `cos`, `sin`, `tan`, `acos`, `asin`, `atan`; the usual trigonometric functions (angles in radians).
  - `cosh`, `sinh`, `tanh`, `acosh`, `asinh`, `atanh`; the usual hyperbolic functions.
  - `log`, a log-transformation.
  - `round`, rounding to a certain number of decimal places.
  - `signif`, rounding to a certain number of significant digits.

Optionally, if `method = "log"`, the operation's group may contain:

- `base`, a scalar dataset containing the base of the log-transformation.
  If this is absent, it is assumed that the natural base will be used.
  The exact float representation is left to the implementation.

If `method = "round"` or `"signif"`, the operation's group should contain:

- `digits`, a scalar integer dataset specifying the number of digits to report.
  The exact integer representation is left to the implementation.

## Shape

The shape of the unary math operation's output object is the same as that of the `seed` delayed object.

## Type

The value type of the unary math operation's output object is usually float.
The only exceptions are for `abs`, where the value type is either integer or float depending on the input (booleans are promoted to integer);
and `sign`, where the value type is always integer.
