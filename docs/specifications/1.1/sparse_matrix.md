

# Sparse matrix (1.1)

## Overview 

A sparse matrix is stored in the compressed sparse layout,
using the same notation as the [10X Genomics feature barcode matrices](https://support.10xgenomics.com/single-cell-gene-expression/software/pipelines/latest/advanced/h5_matrices).
Both compressed sparse column (CSC) and compressed sparse row (CSR) matrices are supported.

## Contents

A sparse matrix is represented as a HDF5 group with the following attributes:

- `delayed_type` should be a scalar string dataset that contains `"array"`.
- `delayed_array` should be a scalar string dataset that contains `"sparse matrix"`.

Inside the matrix's group, we expect:

- A `shape` 1-dimensional integer dataset, containing the dimensions of the matrix.
  The datatype should be representable by a 64-bit unsigned integer.
- A `data` 1-dimensional dataset, containing the values of the structural non-zero elements.
  This should have a `type` scalar string attribute that specifies the value type of the array.
  The attribute should contain one of the following values:
  - `"INTEGER"`, in which case `data` should have a datatype that fits into a 32-bit signed integer.
  - `"FLOAT"`, in which case `data` should have a datatype that fits into a 64-bit float.
  - `"BOOLEAN"`, in which case `data` should have a datatype that fits into a 8-bit signed integer.
- An `indices` 1-dimensional dataset, 
  containing the row indices (CSC) or column indices (CSR) for the structural non-zero elements.
  This should have the same length as the `data` dataset.
  Each entry should be an integer in $[0, X)$ where $X$ is the number of rows (CSC) or columns (CSR) from `shape`.
  The datatype should be representable by a 64-bit unsigned integer.
- An `indptr` 1-dimensional dataset, containing pointers into the sequence of structural non-zero elements in the `indices` and `data` datasets.
  This should have length equal to $Y + 1$  where $Y$ is the number of columns (CSC) or rows (CSR) from `shape`.
  The first element of `indptr` should be equal to zero, the last element should be equal to the length of `data`, and entries should be non-strictly increasing.
  The datatype should be representable by a 64-bit unsigned integer.
- A `by_column` scalar dataset, indicating whether this is a CSC matrix.
  This should have a datatype that is representable by an 8-bit signed integer.
  A non-zero value indicates that the matrix is CSC, otherwise it is treated as CSR.

The structural non-zero elements belonging to the $i$-th column (for CSC) or row (for CSR) are located by the slice $[P_i, P_{i+1})$,
where $P_i$ and $P_{i+1}$ are the $i$-th and $i+1$-th elements of the `indptr` dataset, respectively.
These should be interpreted as pointers into the `indices` and `data` datasets,
i.e., the first structural non-zero element of row/column $i$ is the $P_i$-th entry of those datasets,
while the $P_{i+1}$-th entry is the position immediately after the last structural non-zero element of that row/column.
The indices of the structural non-zeros for a given row/column should be sorted in strictly increasing order,
i.e., $I_{P_i} < I_{P_i + 1} < ... < I_{P_{i+1} - 1}$ where $I_j$ is the $j$-th entry of the `indices` dataset.

Optionally, the matrix's group may contain:

- A `dimnames` group, representing a [list](_general.md#lists) of length 2.
  If a child object exists at `dimnames/{i}`, it should be a 1-dimensional string dataset of length equal to the extent of the $i$-th dimension of `seed`, 
  e.g., `dimnames/0` contains the names for the first dimension.
  Each entry of `dimnames/{i}` contains a name of the corresponding element of the $i$-th dimension of `seed`.
  If no child object is present at `dimnames/{i}`, the $i$-th dimension is unnamed.

`data` may contain a `missing_placeholder` attribute.
This should be a scalar attribute of the exact same datatype as `data`. 
The value of this attribute is the missing placeholder used for all missing elements,
i.e., any elements in ` data ` with the same value as the placeholder should be treated as missing.

## Shape

The shape of the sparse matrix is defined as the contents of `dimensions`.

## Type

The value type of the sparse matrix is set to the `type` attribute of `data`.
