

# Sparse matrix (0.99)

## Overview 

A sparse matrix is stored in the compressed sparse column (CSC) layout,
using the same notation as the [10X Genomics feature barcode matrices](https://support.10xgenomics.com/single-cell-gene-expression/software/pipelines/latest/advanced/h5_matrices).


## Contents

A sparse matrix is represented as a HDF5 group with the following attributes:

- `delayed_type` should be a scalar string dataset that contains `"array"`.
- `delayed_array` should be a scalar string dataset that contains `"sparse matrix"`.

Inside the matrix's group, we expect:

- A `shape` 1-dimensional integer dataset, containing the dimensions of the matrix.
  The exact integer representation is left to the implementation.
- A `data` 1-dimensional dataset, containing the values of the structural non-zero elements.
  This can be integer or float; the exact datatype is left to the implementation.
  Optionally, if `data` is an integer dataset, it may contain an `is_boolean` scalar integer attribute.
  A non-zero value of `is_boolean` indicates that the contents of `data` should be treated as booleans.
- An `indices` 1-dimensional dataset, 
  containing the row indices for the structural non-zero elements.
  This should have the same length as the `data` dataset.
  Each entry should be an integer in $[0, X)$ where $X$ is the number of rows from `shape`.
  The exact integer representation is left to the implementation.
- An `indptr` 1-dimensional dataset, containing pointers into the sequence of structural non-zero elements in the `indices` and `data` datasets.
  This should have length equal to $Y + 1$ where $Y$ is the number of columns from `shape`.
  The first element of `indptr` should be equal to zero, the last element should be equal to the length of `data`, and entries should be non-strictly increasing.
  The exact integer representation is left to the implementation.


The structural non-zero elements belonging to the $i$-th column are located by the slice $[P_i, P_{i+1})$,
where $P_i$ and $P_{i+1}$ are the $i$-th and $i+1$-th elements of the `indptr` dataset, respectively.
These should be interpreted as pointers into the `indices` and `data` datasets,
i.e., the first structural non-zero element of column $i$ is the $P_i$-th entry of those datasets,
while the $P_{i+1}$-th entry is the position immediately after the last structural non-zero element of that column.
The indices of the structural non-zeros for a given column should be sorted in strictly increasing order,
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

The value type of the sparse matrix is inferred from the datatype of `data`: integer, float or string.
Boolean matrices are identified as those with integer `data` and a truthy `is_boolean` flag.
