# This tests various odds and ends not covered by other tests.
# library(testthat); library(chihaya); source("test-simple.R")

library(S4Vectors)
library(Matrix)

test_that("saving of an array works correctly", {
    x0 <- matrix(runif(200), ncol=20)
    x <- DelayedArray(x0)
    tmp <- tempfile(fileext=".h5")
    saveDelayed(x, tmp)

    arra <- rhdf5::h5read(tmp, "delayed/data")
    expect_identical(x0, arra)
    out <- loadDelayed(tmp)
    expect_identical(x, out)

    # Handles dimnames.
    dimnames(x0) <- list(1:nrow(x), head(letters, ncol(x)))
    x <- DelayedArray(x0)

    tmp <- tempfile(fileext=".h5")
    saveDelayed(x, tmp)

    expect_identical(as.vector(rhdf5::h5read(tmp, "delayed/dimnames/0")), rownames(x))
    expect_identical(as.vector(rhdf5::h5read(tmp, "delayed/dimnames/1")), colnames(x))

    out <- loadDelayed(tmp)
    expect_identical(x, out)

    # Handles some missing dimnames.
    rownames(x0) <- NULL
    x <- DelayedArray(x0)

    tmp <- tempfile(fileext=".h5")
    saveDelayed(x, tmp)

    out <- loadDelayed(tmp)
    expect_identical(x, out)
})

test_that("saving of a logical array works correctly", {
    x0 <- matrix(runif(200), ncol=20)

    # Preserves logical arrays.
    x <- DelayedArray(x0 > 0.5)
    tmp <- tempfile(fileext=".h5")
    saveDelayed(x, tmp)

    arra <- rhdf5::h5readAttributes(tmp, "delayed/data")
    expect_identical(1L, arra$is_boolean)
    out <- loadDelayed(tmp)
    expect_identical(x, out)
})

test_that("missing values in character arrays are respected", {
    x0 <- matrix(sample(LETTERS, 100, replace=TRUE), 5, 20)
    x <- DelayedArray(x0)
    tmp <- tempfile(fileext=".h5")
    saveDelayed(x, tmp)

    arra <- rhdf5::h5readAttributes(tmp, "delayed/data")
    expect_null(arra[["missing-value-placeholder"]])
    out <- loadDelayed(tmp)
    expect_identical(x, out)

    # Throwing in some missing values.
    x0[1] <- "NA"
    x0[100] <- NA
    x <- DelayedArray(x0)
    tmp <- tempfile(fileext=".h5")
    saveDelayed(x, tmp)

    arra <- rhdf5::h5readAttributes(tmp, "delayed/data")
    expect_identical("_NA", arra[["missing-value-placeholder"]])
    out <- loadDelayed(tmp)
    expect_identical(x, out)
})

test_that("saving of a CsparseMatrix works correctly", {
    x0 <- rsparsematrix(20, 10, 0.1)
    x <- DelayedArray(x0)
    tmp <- tempfile(fileext=".h5")
    saveDelayed(x, tmp)

    # Check that it follows H5SparseMatrix conventions.
    library(HDF5Array)
    stuff <- H5SparseMatrix(tmp, "delayed")
    expect_identical(unname(as.matrix(stuff)), unname(as.matrix(x)))

    out <- loadDelayed(tmp)
    expect_identical(x, out)

    # Supports dimnames.
    dimnames(x0) <- list(LETTERS[1:20], letters[1:10])
    x <- DelayedArray(x0)
    tmp <- tempfile(fileext=".h5")
    saveDelayed(x, tmp)

    out <- loadDelayed(tmp)
    expect_identical(x, out)
})

test_that("saving of a CsparseMatrix works correctly with integers", {
    x0 <- round(rsparsematrix(20, 10, 0.1) * 10)
    x <- DelayedArray(x0)
    tmp <- tempfile(fileext=".h5")
    saveDelayed(x, tmp)

    library(HDF5Array)
    stuff <- H5SparseMatrix(tmp, "delayed")
    expect_identical(type(stuff), "integer")
    expect_equal(unname(as.matrix(stuff)), unname(as.matrix(x)))

    out <- loadDelayed(tmp)
    expect_identical(x, out)

    # Trying with larger integers.
    x <- DelayedArray(x0 * 10000)
    tmp <- tempfile(fileext=".h5")
    saveDelayed(x, tmp)
    out <- loadDelayed(tmp)
    expect_identical(x, out)
})

test_that("saving of a CsparseMatrix works correctly with logicals", {
    x0 <- rsparsematrix(20, 10, 0.1) != 0
    x <- DelayedArray(x0)
    tmp <- tempfile(fileext=".h5")
    saveDelayed(x, tmp)

    arra <- rhdf5::h5readAttributes(tmp, "delayed/data")
    expect_identical(1L, arra$is_boolean)

    out <- loadDelayed(tmp)
    expect_identical(x, out)
})

test_that("type chooser works correctly", {
    expect_identical(chihaya:::get_best_type(c(1.2, 2.3)), "H5T_NATIVE_DOUBLE")
    expect_identical(chihaya:::get_best_type(c(1, 2)), "H5T_NATIVE_USHORT")
    expect_identical(chihaya:::get_best_type(c(-1, 2)), "H5T_NATIVE_SHORT")
    expect_identical(chihaya:::get_best_type(c(100000)), "H5T_NATIVE_UINT")
    expect_identical(chihaya:::get_best_type(c(-100000)), "H5T_NATIVE_INT")
    expect_identical(chihaya:::get_best_type(numeric(0)), "H5T_NATIVE_USHORT")
    expect_identical(chihaya:::get_best_type(c(5e9)), "H5T_NATIVE_ULONG")
    expect_identical(chihaya:::get_best_type(c(-5e9)), "H5T_NATIVE_LONG")
})
