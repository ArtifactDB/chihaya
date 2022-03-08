# Delayed operations in HDF5

This repository contains a specification for delayed array operations stored in a HDF5 file.
The aim idea is to save the delayed operations to avoid duplication of data, by holding references to the untouched originals in user-defined databases.
The same approach can also be used to delay operations that would otherwise incur a loss of sparsity.

We implement a C++ library for cross-language validation of delayed operations.
At some point, we may also add [**tatami**](https://github.com/LTLA/tatami) bindings to load the delayed operations into memory.
The library is provisionally named after [Chihaya Kisaragi](https://myanimelist.net/character/10369/Chihaya_Kisaragi), one of my favorite characters.

![Chihaya GIF](https://raw.githubusercontent.com/LTLA/acceptable-anime-gifs/master/registry/10278_Idolmaster/0001.gif)
