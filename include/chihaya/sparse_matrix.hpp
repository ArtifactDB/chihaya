#ifndef CHIHAYA_SPARSE_MATRIX_HPP
#define CHIHAYA_SPARSE_MATRIX_HPP

#include "H5Cpp.h"
#include "ritsuko/ritsuko.hpp"

#include <vector>
#include <stdexcept>
#include <cstdint>
#include <cstddef>
#include <string>

#include "utils_public.hpp"
#include "utils_misc.hpp"
#include "utils_type.hpp"
#include "utils_dimensions.hpp"

/**
 * @file sparse_matrix.hpp
 * @brief Validation for compressed sparse column matrices. 
 */

namespace chihaya {

/**
 * @cond
 */
template<typename Index_>
void validate_sparse_indices(const H5::DataSet& ihandle, const std::vector<std::uint64_t>& indptrs, std::size_t primary, std::size_t secondary, bool csc) {
    ritsuko::hdf5::Stream1dNumericDataset<Index_> stream(&ihandle, sanisizer::cast<hsize_t>(indptrs.back()));
    auto buffer = sanisizer::create<std::vector<Index_> >(stream.chunk_size());

    hsize_t available = 0, at = 0;
    auto next = [&]() -> Index_ {
        if (at == available) {
            at = 0;
            available = stream.load(buffer.data());
        }
        return buffer[at++];
    };

    for (std::size_t p = 0; p < primary; ++p) {
        const auto start = indptrs[p];
        const auto end = indptrs[p + 1];
        if (start > end) {
            throw std::runtime_error("entries of 'indptr' must be sorted");
        }
        if (start == end) {
            continue;
        }

        Index_ previous = next();
        if (previous < 0) {
            throw std::runtime_error("entries of 'indices' should be non-negative");
        }

        // If it's sorted in strictly increasing order, we only need to check the first entry for negative values.
        // Similarly, we only need to check the last entry for whether it exceeds the secondary limit.
        for (I<decltype(start)> x = start + 1; x < end; ++x) {
            const auto i = next();
            if (i <= previous) {
                throw std::runtime_error("'indices' should be strictly increasing within each " + (csc ? std::string("column") : std::string("row")));
            }
            previous = i;
        }

        if (sanisizer::is_greater_than_or_equal(previous, secondary)) {
            throw std::runtime_error("entries of 'indices' should be less than the number of " + (csc ? std::string("row") : std::string("column")) + "s");
        }
    }
}
/**
 * @endcond
 */

/**
 * @param handle An open handle on a HDF5 group representing a sparse matrix.
 * @param version Version of the **chihaya** specification.
 * @param options Validation options.
 * 
 * @return Details of the sparse matrix.
 * Otherwise, if the validation failed, an error is raised.
 */
inline ArrayDetails validate_sparse_matrix(const H5::Group& handle, const ritsuko::Version& version, Options& options) {
    std::vector<std::size_t> dims;
    ArrayType array_type;

    {
        auto shandle = handle.openDataSet("shape");
        auto sspace = shandle.getSpace();
        if (sspace.getSimpleExtentNdims() != 1) {
            throw std::runtime_error("'shape' dataset should be 1-dimensional");
        }
        hsize_t len;
        sspace.getSimpleExtentDims(&len);
        if (len != 2) {
            throw std::runtime_error("'shape' dataset should have length 2");
        }

        if (version.lt(1, 1, 0)) {
            dims = load_non_negative_integer_vector_0_99<std::size_t>(shandle, 2);
        } else {
            if (ritsuko::hdf5::exceeds_integer_limit(shandle, 64, false)) {
                throw std::runtime_error("'shape' should have a datatype that can fit into a 64-bit unsigned integer");
            }
            dims = load_dimensions_from_uint64_contents<std::size_t>(shandle, 2);
        }
    }

    hsize_t nnz;
    {
        auto dhandle = handle.openDataSet("data");
        auto dspace = dhandle.getSpace();
        if (dspace.getSimpleExtentNdims() != 1) {
            throw std::runtime_error("'data' dataset should be 1-dimensional");
        }
        dspace.getSimpleExtentDims(&nnz);

        if (version.lt(1, 1, 0)) {
            array_type = translate_type_0_99(dhandle.getTypeClass());
            if (is_boolean_0_99(dhandle)) {
                array_type = BOOLEAN;
            }
        } else {
            auto type = load_scalar_string_attribute(handle, "type");
            array_type = translate_type_1_1(type);
            if (!options.details_only) {
                check_type_1_1(dhandle, array_type);
            }
        }

        if (!options.details_only) {
            if (array_type != INTEGER && array_type != BOOLEAN && array_type != FLOAT) {
                throw std::runtime_error("dataset should be integer, float or boolean");
            }
            validate_missing_placeholder(dhandle, version);
        }
    }

    if (!options.details_only) {
        bool csc = true;
        if (!version.lt(1, 1, 0)) {
            auto bhandle = handle.openDataSet("by_column");
            if (bhandle.getSpace().getSimpleExtentNdims() != 0) {
                throw std::runtime_error("'by_column' dataset should be scalar");
            }
            if (ritsuko::hdf5::exceeds_integer_limit(bhandle, 8, true)) {
                throw std::runtime_error("datatype of the 'by_column' dataset should fit into an 8-bit signed integer");
            }
            std::int8_t val;
            bhandle.read(&val, H5::PredType::NATIVE_INT8);
            csc = (val != 0);
        }

        const auto primary = (csc ? dims[1] : dims[0]);
        const auto secondary = (csc ? dims[0] : dims[1]);

        std::vector<std::uint64_t> indptrs;
        {
            auto iphandle = handle.openDataSet("indptr");
            auto ipspace = iphandle.getSpace();
            if (ipspace.getSimpleExtentNdims() != 1) {
                throw std::runtime_error("'indptr' dataset should be 1-dimensional");
            }
            hsize_t iplen;
            ipspace.getSimpleExtentDims(&iplen);

            if (iplen == 0 || !sanisizer::is_equal(iplen - 1, primary)) { // avoid risk of potential overflow with primary + 1.
                throw std::runtime_error("'indptr' should have length equal to the number of " + (csc ? std::string("columns") : std::string("rows")) + " plus 1");
            }

            if (version.lt(1, 1, 0)) {
                if (iphandle.getTypeClass() != H5T_INTEGER) {
                    throw std::runtime_error("'indptr' should be integer");
                }
                indptrs = load_non_negative_integer_vector_0_99<std::uint64_t>(iphandle, iplen);
            } else {
                if (ritsuko::hdf5::exceeds_integer_limit(iphandle, 64, false)) {
                    throw std::runtime_error("datatype of 'indptr' should fit into a 64-bit unsigned integer");
                }
                sanisizer::resize(indptrs, iplen);
                iphandle.read(indptrs.data(), H5::PredType::NATIVE_UINT64);
            }

            iphandle.read(indptrs.data(), H5::PredType::NATIVE_UINT64);
            if (indptrs[0] != 0) {
                throw std::runtime_error("first entry of 'indptr' should be 0");
            }
            if (!sanisizer::is_equal(indptrs.back(), nnz)) {
                throw std::runtime_error("last entry of 'indptr' should be equal to the length of 'data'");
            }
        }

        {
            auto ihandle = handle.openDataSet("indices");
            auto ispace = ihandle.getSpace();
            if (ispace.getSimpleExtentNdims() != 1) {
                throw std::runtime_error("'indices' dataset should be 1-dimensional");
            }
            hsize_t inum;
            ispace.getSimpleExtentDims(&inum);
            if (nnz != inum) {
                throw std::runtime_error("'indices' and 'data' should have the same length");
            }

            if (version.lt(1, 1, 0)) {
                if (!ritsuko::hdf5::exceeds_integer_limit(ihandle, 64, true)) {
                    validate_sparse_indices<std::int64_t>(ihandle, indptrs, primary, secondary, csc);
                } else if (!ritsuko::hdf5::exceeds_integer_limit(ihandle, 64, false)) {
                    validate_sparse_indices<std::uint64_t>(ihandle, indptrs, primary, secondary, csc);
                } else {
                    throw std::runtime_error("'indices' should be integer");
                }

            } else {
                if (ritsuko::hdf5::exceeds_integer_limit(ihandle, 64, false)) {
                    throw std::runtime_error("datatype of 'indices' should fit into a 64-bit unsigned integer");
                }
                validate_sparse_indices<std::uint64_t>(ihandle, indptrs, primary, secondary, csc);
            }
        }

        // Validating dimnames.
        if (handle.exists("dimnames")) {
            validate_dimnames_internal(handle, dims, version);
        }
    }

    return ArrayDetails(array_type, std::move(dims));
}

}

#endif
