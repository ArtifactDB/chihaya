

# Sparse matrix (0.99)

## Overview 

A sparse matrix is stored in the compressed sparse column (CSC) layout,
using the same notation as the [10X Genomics feature barcode matrices](https://support.10xgenomics.com/single-cell-gene-expression/software/pipelines/latest/advanced/h5_matrices).


## Specification

A sparse matrix is represented as a HDF5 group with the following attributes:

- `delayed_type` should be a scalar string dataset, of any datatype that can be represented by a UTF-8 encoded string.
  It should contain the value `"array"`.
- `delayed_array` should be a scalar string dataset, of any datatype that can be represented by a UTF-8 encoded string.
  It should contain the value `"sparse matrix"`.

Inside the group, we expect:

- A `shape` 1-dimensional integer dataset, containing the dimensions of the matrix.
  The exact integer representation is left to the implementation.
- A `data` 1-dimensional dataset, containing the values of the non-zero elements.
  This can be integer or float; the exact datatype is left to the implementation.
  If `data` is an integer dataset, it may additionally contain an `is_boolean` attribute.
  This should be an integer scalar; if non-zero, it indicates that the contents of `data` should be treated as booleans where zeros are falsey and non-zeros are truthy.
- An `indices` 1-dimensional dataset, 
  containing the row indices for the non-zero elements.
  This should have the same length as `data` and should contain integers in `[0, X)` where `X` is the number of rows from `shape`.
  Entries should be strictly increasing within each column, based on the ranges defined by `indptr`.
  The exact integer representation is left to the implementation.
- An `indptr` 1-dimensional dataset, containing pointers into the `indices` vector.
  This should have length equal to `Y + 1` where `Y` is the number of columns from `shape`.
  The first element should be equal to zero and the last element should be equal to the length of `data`.
  The exact integer representation is left to the implementation.


The group may also contain:

- A `dimnames` group, representing a [list](_general.md#lists) of length 2.
  Each child entry corresponds to a dimension of `seed` and contains the names along that dimension, e.g., entry `0` contains row names.
  Missing entries indicate that no names are attached to its dimension.
  Each (non-missing) entry should be a 1-dimensional string dataset of length equal to the extent of its dimension.
  The datatype of each dataset should be representable by a UTF-8 encoded string.

`data` may contain a `missing_placeholder` attribute.
This should be a scalar dataset of the exact same datatype as `data`.
The value of this dataset is the missing placeholder used for all missing elements,
i.e., any elements in `data` with the same value as the placeholder should be treated as missing.
Check out the [HDF5 policy draft (v0.1.0)](https://github.com/ArtifactDB/Bioc-HDF5-policy/tree/v0.1.0) for more details.

The value type of the sparse matrix is inferred from the datatype of `data`: integer, float or string.
Boolean matrices are identified as those with integer `data` and a truthy `is_boolean` flag.

