# Delayed operations in HDF5

This repository contains a specification for delayed array operations stored in a HDF5 file.
The concept of delayed operations is taken from Bioconductor's [**DelayedArray**](https://bioconductor.org/packages/DelayedArray) package,
where any operations on a `DelayedArray` are cached in memory and evaluated on an as-needed basis.
Our aim is to save these operations to file in a well-defined, cross-language format;
this avoids the need to compute and store the results of such operations, which may be prohibitively expensive.

Several use cases benefit from the serialization of delayed operations:

- We have an immutable array dataset stored in a database.
  Rather than making a copy for manipulation, we can hold a reference to the original and save the operations (slicing, arithmetic, etc.).
  This avoids duplication of large datasets.
- We have a dataset that can be represented in a small type, e.g., `uint8_t`s.
  We apply a transformation that promotes the type, e.g., log-transformation to `float`s or `double`s.
  By saving the delayed operation, we can maintain our compact representation to reduce the file size.
- We have a sparse dataset that is subjected to sparsity-breaking operation, e.g., centering.
  Rather than saving the dense matrix, we keep the efficient sparse representation and save the delayed operation.
 
A "delayed object" is stored in the file as a HDF5 group with the `delayed_type` string attribute.
This type can either be `"operation"`, in which case the object represents a delayed operation that is applied to a simpler (nested) "seed" object;
or it can be an `"array"`, in which case the object represents an array prior to any application of operations. 

To enforce the **chihaya** specification, we implement a C++ library for cross-language validation of each delayed operation.
The specification for each operation is defined in terms of the documentation for its validator function:

- [Subsetting](https://ltla.github.io/chihaya/subset_8hpp.html)
- [Combining](https://ltla.github.io/chihaya/combine_8hpp.html)
- [Transposition](https://ltla.github.io/chihaya/transpose_8hpp.html)
- [Dimnames assignment](https://ltla.github.io/chihaya/dimnames_8hpp.html)
- [Subset assignment](https://ltla.github.io/chihaya/subset__assignment_8hpp.html)
- [Unary arithmetic](https://ltla.github.io/chihaya/unary__arithmetic_8hpp.html)
- [Unary comparison](https://ltla.github.io/chihaya/unary__comparison_8hpp.html)
- [Unary logic](https://ltla.github.io/chihaya/unary__logic_8hpp.html)
- [Unary math](https://ltla.github.io/chihaya/unary__math_8hpp.html)
- [Unary special checks](https://ltla.github.io/chihaya/unary__special__checks_8hpp.html)
- [Binary arithmetic](https://ltla.github.io/chihaya/binary__arithmetic_8hpp.html)
- [Binary comparison](https://ltla.github.io/chihaya/binary__comparison_8hpp.html)
- [Binary logic](https://ltla.github.io/chihaya/binary__logic_8hpp.html)

Similar validators are available for the arrays:

- [Dense arrays](https://ltla.github.io/chihaya/dense__array_8hpp.html)
- [Sparse matrices](https://ltla.github.io/chihaya/sparse__matrix_8hpp.html)
- [External arrays](https://ltla.github.io/chihaya/external_8hpp.html)

At some point, we may also add [**tatami**](https://github.com/LTLA/tatami) bindings to load the delayed operations into memory.
This would enable C++ applications to natively read from the HDF5 files that comply with **chihaya**'s specification.

The library is provisionally named after [Chihaya Kisaragi](https://myanimelist.net/character/10369/Chihaya_Kisaragi), one of my favorite characters.

![Chihaya GIF](https://raw.githubusercontent.com/LTLA/acceptable-anime-gifs/master/registry/10278_Idolmaster/0001.gif)