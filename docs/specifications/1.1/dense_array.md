

# Dense array (1.1)

## Overview 

A dense array is a... dense array.

## Contents 

A dense array is represented as a HDF5 group with the following attributes:

- `delayed_type` should be a scalar string dataset that contains `"array"`.
- `delayed_array` should be a scalar string dataset that contains `"dense array"`.

Inside the array's group, we expect:

- A `data` dataset, containing the array data.
  This should have a non-zero number of dimensions (i.e., not scalar).
  `data` should have a `type` scalar string attribute that specifies the value type of the array.
  This attribute should contain one of the following values:
  - `"INTEGER"`, in which case `data` should have a datatype that fits into a 32-bit signed integer.
  - `"FLOAT"`, in which case `data` should have a datatype that fits into a 64-bit float.
  - `"BOOLEAN"`, in which case `data` should have a datatype that fits into a 8-bit signed integer.
  - `"STRING"`, in which case `data` should have a string datatype.
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
This should be a scalar attribute of the exact same datatype as `data`. 
The value of this attribute is the missing placeholder used for all missing elements,
i.e., any elements in ` data ` with the same value as the placeholder should be treated as missing.

Optionally, the array's group may contain:

- A `dimnames` group, representing a [list](_general.md#lists) of length equal to the number of dimensions in `data`.
  If a child object exists at `dimnames/{i}`, it should be a 1-dimensional string dataset of length equal to the extent of the $i$-th dimension of `data`. 
  e.g., `dimnames/0` contains the names for the first dimension.
  Each entry of `dimnames/{i}` contains a name of the corresponding element of the $i$-th dimension of `seed`.
  If no child object is present at `dimnames/{i}`, the $i$-th dimension is unnamed.

Note that the names of the datasets inside `dimnames` refer to the dimensions of `data`, not of the dense array.
This distinction is important with `native = 0`, e.g., readers should only perform the transposition after applying the names to each dimension of `data`.

## Shape

If `native` is truthy, the shape of the dense array is defined as the dimensions of the `data` dataset.
Otherwise, the shape of the dense array is defined by reversing the dimensions of `data`.

## Type

The value type of the dense array is set to the `type` attribute of `data`.
