
- `delayed_type` should be a scalar string dataset, of any datatype that can be represented by a UTF-8 encoded string.
  It should contain the value `"array"`.
- `delayed_array` should be a scalar string dataset, of any datatype that can be represented by a UTF-8 encoded string.
  It should contain the value `"constant array"`.



# Dense array (1.1)

## Overview 

A dense array is a... dense array.

## Specification

A dense array is represented as a HDF5 group with the following attributes:

- `delayed_type` should be a scalar string dataset, of any datatype that can be represented by a UTF-8 encoded string.
  It should contain the value `"array"`.
- `delayed_array` should be a scalar string dataset, of any datatype that can be represented by a UTF-8 encoded string.
  It should contain the value `"dense array"`.

Inside the group, we expect:

- A `data` dataset, containing the array data.
  This should have a non-zero number of dimensions (i.e., not scalar).
  `data` should have a `type` scalar string attribute that specifies the value type of the array.
  This attribute should of a datatype that can be represented by a UTF-8 encoded string, and should hold one of the following values:
  - `"INTEGER"`, in which case `data` should have a datatype that fits into a 32-bit signed integer.
  - `"FLOAT"`, in which case `data` should have a datatype that fits into a 64-bit float.
  - `"BOOLEAN"`, in which case `data` should have a datatype that fits into a 8-bit signed integer.
  - `"STRING"`, in which case `data` should have a datatype that can be represented by a UTF-8 encoded string.
- A `native` scalar dataset, to be interpreted as a boolean.
  This should have a datatype that fits into a 8-bit signed integer, where non-zero values are treated as truthy.
  The value specifies whether the dimensions of the dense array are sorted from slowest-changing (first) to fastest-changing (last).
  If true, the dimensions of the `data` dataset are in the same order as the dimensions of the dense array.
  If false, the dimensions are saved in reverse order, i.e., the first dimension of the dense array is the last dimension of the `data` dataset.

Setting `native = 0` is frequently done for efficiency when the in-memory dense array has a different layout than the on-disk HDF5 dataset.
For example, Fortran, R and Julia use column-major order for their matrices, while C code (and HDF5) would typically use row-major order.
By setting `native = 0`, we avoid the need to reorganize the data when reading/writing from file;
however, this means that the dimensions reported by HDF5 need to be reversed to obtain the dimensions of the delayed object.

`data` may contain a `missing_placeholder` attribute.
This should be a scalar dataset of the exact same datatype as `data`.
The value of this dataset is the missing placeholder used for all missing elements,
i.e., any elements in `data` with the same value as the placeholder should be treated as missing.

The group may also contain:

- A `dimnames` group, representing a [list](_general.md#lists) of length equal to the number of dimensions in the dense array.
  Each child entry corresponds to a dimension of `data` and contains the names along that dimension, e.g., entry `0` contains names for the first dimension of `data`.
  The absence of a child entry indicates that no names are attached to the corresponding dimension.
  Each (non-absent) entry should be a 1-dimensional string dataset of length equal to the extent of its dimension.
  The datatype of each dataset should be representable by a UTF-8 encoded string.

The value type of the dense array is set to the `type` attribute of `data`.
