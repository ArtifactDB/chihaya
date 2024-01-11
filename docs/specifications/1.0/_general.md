

# General concepts (1.0)

## Overview 

We consider the idea of a "delayed object", which is some N-dimensional array-like object of a certain [value type](#value-type).
Each delayed object consists of zero, one or more [delayed operations](#operation-types) applied to one or more [arrays](#array-types) containing the original data.
Applying a delayed operation to a delayed object yields a new delayed object, possibly of different dimensionality and/or value type.
An array without any operation is also a delayed object, albeit a rather uninteresting one.

In the **chihaya** specification, we represent a delayed object inside a HDF5 file as a group.
If the delayed object was constructed from a delayed operation, the most recent operation is specified in the group's attributes;
the "seed" (i.e., the delayed object to which the operation was applied) is then saved as a subgroup.
This is done recursively until we save the underlying array of original data.

By inspecting the **chihaya** HDF5 file, readers can reconstitute the entire delayed object by re-applying all of the operations to the array.
For example, in the following HDF5 file structure, we start with a dense array, take the absolute value of each element and then add 2 to obtain the final delayed object.
The same approach is used for arbitrarily complex "trees" of delayed operations, possibly involving multiple arrays (e.g., via a delayed combine).

```
/
└─ hello_world 
   ├─ ATTRIBUTE: delayed_type = "operation"
   ├─ ATTRIBUTE: delayed_operation = "unary arithmetic"
   ├─ DATASET: method = "+"
   ├─ DATASET: side = "right"
   ├─ DATASET: value = 2
   |  └─ ATTRIBUTE: type = "FLOAT"
   └─ GROUP: seed
      ├─ ATTRIBUTE: delayed_type = "operation"
      ├─ ATTRIBUTE: delayed_operation = "unary math"
      ├─ DATASET: method = "abs"
      └─ GROUP: seed
         ├─ ATTRIBUTE: delayed_type = "array"
         ├─ ATTRIBUTE: delayed_array = "dense array"
         └─ DATASET: data = [ 10 x 4 ]
            └─ ATTRIBUTE: type = "INTEGER"
```

## Operation types

Each operation is represented by a HDF5 group with the `delayed_type` attribute set to `"operation"`.
This requires an additional `delayed_operation` string attribute to specify the type of operation.
The following operation types are supported by **chihaya**:

- [Subsetting](subset.md)
- [Combining](combine.md)
- [Transposition](transpose.md)
- [Dimnames assignment](dimnames.md)
- [Subset assignment](subset_assignment.md)
- [Unary arithmetic](unary_arithmetic.md)
- [Unary comparison](unary_comparison.md)
- [Unary logic](unary_logic.md)
- [Unary math](unary_math.md)
- [Unary special checks](unary_special_check.md)
- [Binary arithmetic](binary_arithmetic.md)
- [Binary comparison](binary_comparison.md)
- [Binary logic](binary_logic.md)
- [Matrix product](matrix_product.md)

It is expected that the group contains at least one subgroup that represents the seed to which the operation was applied.
Note that the value type and dimensions of the post-operation object need not be the same as that of the seed(s).

## Array types

Each operation is represented by a HDF5 group with the `delayed_type` attribute set to `"array"`.
This requires an additional `delayed_array` string attribute to specify the type of array.
Several array types are supported:

- [Dense arrays](dense_array.md)
- [Sparse matrices](sparse_matrix.md)
- [Constant arrays](constant_array.md)
- [Custom arrays](custom_array.md)
- [External HDF5 arrays](external_hdf5.md)

Each instance of an array type should have dimensions and a well-defined value type.
Each delayed object ultimately terminates in one or more arrays containing or referencing the pre-operation data.

## Value types

Delayed objects can have four value types - boolean, integer, float and string.
Applying a delayed operation to an object may cause it to change its type.

Booleans are promoted to integers via the usual rules, i.e., true values are 1 and false values are 0.

Integers are promoted to floats in the expected manner.
As a general rule, the framework that reads **chihaya** files should use 32-bit signed integers and 64-bit floats.
Thus, any promotion of integers to floats will be lossless.

In some cases, integers or floats are treated as booleans.
This casting is done in the usual way, i.e., non-zero values are truthy and zero values are falsey.



## Lists

Some specifications will include a "list", which is represented as a HDF5 group with the following attributes:

- `delayed_type`, a scalar string dataset with a datatype that can be represented by a UTF-8 encoded string.
  This should contain the value `"list"`.
- `delayed_length`, a scalar integer dataset specifying the list length.
  Any integer datatype may be used here.

Children of this group represent the list elements and are named by their positional index.
List elements may be absent, e.g., a group with `delayed_length` set to 3 and the children `0` and `2` represents a list that is missing an element at index 1.
The intepretation of the absence of an element is context-dependent.
