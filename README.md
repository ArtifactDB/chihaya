# Delayed operations in HDF5

## Introduction

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

## Specification

In the **chihaya** specification, we store a "delayed object" as a HDF5 group in the file.
Delayed operations are represented as further nested groups, terminating in an array containing the original data (or a reference to it).
The type of delayed operation/array is specified in the group's attributes.

We have implemented a C++ library for cross-language validation of each delayed operation.
The specification for each delayed operation is defined in the documentation for its validator function:

- [Subsetting](https://artifactdb.github.io/chihaya/subset_8hpp.html)
- [Combining](https://artifactdb.github.io/chihaya/combine_8hpp.html)
- [Transposition](https://artifactdb.github.io/chihaya/transpose_8hpp.html)
- [Dimnames assignment](https://artifactdb.github.io/chihaya/dimnames_8hpp.html)
- [Subset assignment](https://artifactdb.github.io/chihaya/subset__assignment_8hpp.html)
- [Unary arithmetic](https://artifactdb.github.io/chihaya/unary__arithmetic_8hpp.html)
- [Unary comparison](https://artifactdb.github.io/chihaya/unary__comparison_8hpp.html)
- [Unary logic](https://artifactdb.github.io/chihaya/unary__logic_8hpp.html)
- [Unary math](https://artifactdb.github.io/chihaya/unary__math_8hpp.html)
- [Unary special checks](https://artifactdb.github.io/chihaya/unary__special__check_8hpp.html)
- [Binary arithmetic](https://artifactdb.github.io/chihaya/binary__arithmetic_8hpp.html)
- [Binary comparison](https://artifactdb.github.io/chihaya/binary__comparison_8hpp.html)
- [Binary logic](https://artifactdb.github.io/chihaya/binary__logic_8hpp.html)
- [Matrix product](https://artifactdb.github.io/chihaya/matrix__product_8hpp.html)

Similar validators are available for the arrays:

- [Dense arrays](https://artifactdb.github.io/chihaya/dense__array_8hpp.html)
- [Sparse matrices](https://artifactdb.github.io/chihaya/sparse__matrix_8hpp.html)
- [Constant arrays](https://artifactdb.github.io/chihaya/constant__array_8hpp.html)
- [External HDF5 arrays](https://artifactdb.github.io/chihaya/external__hdf5_8hpp.html)
- [Custom arrays](https://artifactdb.github.io/chihaya/custom__array_8hpp.html)

Any number of other arbitrary objects may be stored in the same HDF5 file, as long as these are outside of the group corresponding to the delayed object.

## Using the validation library

In C++, a delayed object in a file can be validated by calling the [`validate`](https://artifactdb.github.io/chihaya/validate_8hpp.html) function:

```cpp
#include "chihaya/chihaya.hpp"

chihaya::validate("path_to_file.h5", "delayed/object/name");
```

In R, `DelayedArray` objects (from the [**DelayedArray**](https://bioconductor.org/packages/DelayedArray) package)
can be saved to a **chihaya**-compliant HDF5 file using the [our R package](https://github.com/AritfactDB/chihaya-R).
The same package also reconstitutes a `DelayedArray` from the file.

```r
library(DelayedArray)
X <- DelayedArray(matrix(runif(100), 100, 20)) 
X <- log(t(t(X) / runif(ncol(X))) + 1) 

library(chihaya)
tmp <- tempfile(fileext=".h5")
saveDelayed(X, tmp)
Y <- loadDelayed(tmp)
```

## Building projects

### CMake with `FetchContent`

If you're using CMake, you just need to add something like this to your `CMakeLists.txt`:

```cmake
include(FetchContent)

FetchContent_Declare(
  chihaya
  GIT_REPOSITORY https://github.com/ArtifactDB/chihaya
  GIT_TAG master # or any version of interest
)

FetchContent_MakeAvailable(chihaya)
```

Then you can link to **chihaya** to make the headers available during compilation:

```cmake
# For executables:
target_link_libraries(myexe chihaya)

# For libaries
target_link_libraries(mylib INTERFACE chihaya)
```

### CMake with `find_package()`

You can install the library by cloning a suitable version of this repository and running the following commands:

```sh
mkdir build && cd build
cmake .. -DTATAMI_TESTS=OFF
cmake --build . --target install
```

Then you can use `find_package()` as usual:

```cmake
find_package(chihaya_chihaya CONFIG REQUIRED)
target_link_libraries(mylib INTERFACE chihaya::chihaya)
```

### Manual

If you're not using CMake, the simple approach is to just copy the files the `include/` subdirectory -
either directly or with Git submodules - and include their path during compilation with, e.g., GCC's `-I`.
You will also need to link to the HDF5 library.

## Further comments

Web applications can read delayed matrices into memory using the [**chihaya**](https://npmjs.com/package/chihaya) Javascript package.

At some point, we may also add [**tatami**](https://github.com/tatami-inc/tatami) bindings to load the delayed operations into memory.
This would enable C++ applications to natively read from the HDF5 files that comply with **chihaya**'s specification.

The library is provisionally named after [Chihaya Kisaragi](https://myanimelist.net/character/10369/Chihaya_Kisaragi), one of my favorite characters.

![Chihaya GIF](https://raw.githubusercontent.com/LTLA/acceptable-anime-gifs/master/registry/10278_Idolmaster/0001.gif)
