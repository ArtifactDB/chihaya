

# Sparse matrix (1.1)

## Overview 

A sparse matrix is stored in the compressed sparse layout,
using the same notation as the [10X Genomics feature barcode matrices](https://support.10xgenomics.com/single-cell-gene-expression/software/pipelines/latest/advanced/h5_matrices).
Both compressed sparse column (CSC) and compressed sparse row (CSR) matrices are supported.

## Specification

A sparse matrix is represented as a HDF5 group with the following attributes:

- `delayed_type` should be a scalar string dataset, of any datatype that can be represented by a UTF-8 encoded string.
  It should contain the value `"array"`.
- `delayed_array` should be a scalar string dataset, of any datatype that can be represented by a UTF-8 encoded string.
  It should contain the value `"sparse matrix"`.

Inside the group, we expect:

- A `shape` 1-dimensional integer dataset, containing the dimensions of the matrix.
  The datatype should be exactly represented by a 64-bit unsigned integer.
- A `data` 1-dimensional dataset, containing the values of the non-zero elements.
  This should have a `type` scalar string attribute that specifies the value type of the array.
  The attribute should be of a datatype that can be represented by a UTF-8 encoded string, and should hold one of the following values:
  - `"INTEGER"`, in which case `data` should have a datatype that fits into a 32-bit signed integer.
  - `"FLOAT"`, in which case `data` should have a datatype that fits into a 64-bit float.
  - `"BOOLEAN"`, in which case `data` should have a datatype that fits into a 8-bit signed integer.
- An `indices` 1-dimensional dataset, 
  containing the row indices (CSC) or column indiecs (CSR) for the non-zero elements.
  This should have the same length as `data` and should contain integers in `[0, X)` where `X` is the number of rows (CSC) or columns (CSR) from `shape`.
  Entries should be strictly increasing within each column, based on the ranges defined by `indptr`.
  The datatype should be exactly represented by a 64-bit unsigned integer.
- An `indptr` 1-dimensional dataset, containing pointers into the `indices` vector.
  This should have length equal to `Y + 1` where `Y` is the number of columns (CSC) or rows (CSR) from `shape`.
  The first element should be equal to zero and the last element should be equal to the length of `data`.
  The datatype should be exactly represented by a 64-bit unsigned integer.
- A `by_column` scalar dataset, indicating whether this is a CSC matrix.
  This should have a datatype that can be exactly represented by an 8-bit signed integer.
  A non-zero value indicates that the matrix is CSC, otherwise it is treated as CSR.

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

The value type of the sparse matrix is set to the `type` attribute of `data`.

