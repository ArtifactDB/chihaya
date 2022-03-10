# Delayed operations in HDF5

This repository contains a specification for delayed array operations stored in a HDF5 file.
The aim is to save the delayed operations in HDF5 to avoid duplication of data, by holding references to the untouched originals in user-defined databases.
The same approach can also be used to delay operations that would otherwise incur a loss of sparsity.

A "delayed object" is stored in the file as a HDF5 group with the `delayed_type` string attribute.
This type can either be `"operation"`, in which case the object represents a delayed operation that is applied to a simpler (nested) "seed" object;
or it can be an `"array"`, in which case the object represents an array prior to any application of operations. 

We implement a C++ library for cross-language validation of each delayed operation.
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
- [External arrays](https://ltla.github.io/chihaya/external_8hpp.html)

At some point, we may also add [**tatami**](https://github.com/LTLA/tatami) bindings to load the delayed operations into memory.
The library is provisionally named after [Chihaya Kisaragi](https://myanimelist.net/character/10369/Chihaya_Kisaragi), one of my favorite characters.

![Chihaya GIF](https://raw.githubusercontent.com/LTLA/acceptable-anime-gifs/master/registry/10278_Idolmaster/0001.gif)
