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
By recursively inspecting the contents of each HDF5 group, applications can reconstitute the original delayed object in the framework of choice.

The **chihaya** specification currently supports a range of delayed operations
including subsetting, combining, transposition, matrix products, and an assortment of unary and binary operations.
It also supports dense arrays, sparse matrices, constant arrays and custom arrays.
More details about the on-disk representation of each operation can be found in the specifications:

- [1.1](https://github.com/ArtifactDB/chihaya/tree/gh-pages/docs/specifications/1.1/_general.md)
- [1.0](https://github.com/ArtifactDB/chihaya/tree/gh-pages/docs/specifications/1.0/_general.md)
- [0.99](https://github.com/ArtifactDB/chihaya/tree/gh-pages/docs/specifications/0.99/_general.md)

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

By default, this will use `FetchContent` to fetch external dependencies.
Applications are advised to pin the versions of all dependencies themselves - see [`extern/CMakeLists.txt`](extern/CMakeLists.txt) for suggested versions.
If you want to install them manually, use `-DCHIHAYA_FETCH_EXTERN=OFF`.

### CMake with `find_package()`

You can install the library by cloning a suitable version of this repository and running the following commands:

```sh
mkdir build && cd build
cmake .. -DTATAMI_TESTS=OFF
cmake --build . --target install
```

Then you can use `find_package()` as usual:

```cmake
find_package(artifactdb_chihaya CONFIG REQUIRED)
target_link_libraries(mylib INTERFACE artifactdb::chihaya)
```

Again, this will use `FetchContent` to fetch dependencies, see comments above.

### Manual

If you're not using CMake, the simple approach is to just copy the files the `include/` subdirectory -
either directly or with Git submodules - and include their path during compilation with, e.g., GCC's `-I`.
This requires the dependencies listed in the [`extern/CMakeLists.txt`](extern/CMakeLists.txt) directory.
You will also need to link to the HDF5 library, usually from a system installation (1.10 or higher).

## Further comments

Web applications can read delayed matrices into memory using the [**chihaya**](https://npmjs.com/package/chihaya) Javascript package.

At some point, we may also add [**tatami**](https://github.com/tatami-inc/tatami) bindings to load the delayed operations into memory.
This would enable C++ applications to natively read from the HDF5 files that comply with **chihaya**'s specification.

The library is provisionally named after [Chihaya Kisaragi](https://myanimelist.net/character/10369/Chihaya_Kisaragi), one of my favorite characters.

![Chihaya GIF](https://raw.githubusercontent.com/LTLA/acceptable-anime-gifs/master/registry/10278_Idolmaster/0001.gif)
