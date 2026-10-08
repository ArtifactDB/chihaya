#ifndef CHIHAYA_SPARSE_MATRIX_HPP
#define CHIHAYA_SPARSE_MATRIX_HPP

#include "H5Cpp.h"
#include "ritsuko/ritsuko.hpp"

#include <vector>
#include <stdexcept>
#include <exception>
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
void validate_sparse_indices(
    const H5::DataSet& ihandle,
    const std::vector<std::uint64_t>& indptrs,
    const std::size_t primary,
    const std::size_t secondary,
    const bool csc,
    const hsize_t contiguous_chunk_size
) {
    ritsuko::hdf5::Stream1dNumericDataset<Index_> stream(
        &ihandle,
        sanisizer::cast<hsize_t>(indptrs.back()),
        [&]{
            ritsuko::hdf5::Stream1dNumericDatasetOptions opt;
            opt.contiguous_chunk_size = contiguous_chunk_size;
            return opt;
        }()
    );
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
        if (start == end) {
            continue;
        }

        Index_ previous = next();
        if (previous < 0) {
            throw std::runtime_error("entries should be non-negative");
        }

        // If it's sorted in strictly increasing order, we only need to check the first entry for negative values.
        // Similarly, we only need to check the last entry for whether it exceeds the secondary limit.
        for (I<decltype(start)> x = start + 1; x < end; ++x) {
            const auto i = next();
            if (i <= previous) {
                throw std::runtime_error("entries should be strictly increasing within each " + (csc ? std::string("column") : std::string("row")));
            }
            previous = i;
        }

        if (sanisizer::is_greater_than_or_equal(previous, secondary)) {
            throw std::runtime_error("entries should be less than the number of " + (csc ? std::string("row") : std::string("column")) + "s");
        }
    }
}
/**
 * @endcond
 */

/**
 * @param group HDF5 group representing a sparse matrix.
 * @param version Version of the **chihaya** specification.
 * @param options Validation options.
 * 
 * @return Details of the sparse matrix.
 *
 * @throw H5::Exception Thrown upon HDF5 library error.
 * This may be wrapped in a `std::nested_exception`.
 * @throw std::exception Thrown upon validation failure.
 * This may be a `std::nested_exception`, in which case the nested exceptions should be extracted to obtain an informative error message.
 */
inline ArrayDetails validate_sparse_matrix(const H5::Group& group, const ritsuko::Version& version, const Options& options) {
    std::vector<std::size_t> dims;
    ArrayType array_type;

    try {
        auto shandle = group.openDataSet("shape");
        auto sspace = shandle.getSpace();
        if (sspace.getSimpleExtentNdims() != 1) {
            throw std::runtime_error("expected a 1-dimensional dataset");
        }
        hsize_t len;
        sspace.getSimpleExtentDims(&len);
        if (len != 2) {
            throw std::runtime_error("dataset should have length 2");
        }

        if (version.lt(1, 1, 0)) {
            dims = load_non_negative_integer_vector_0_99<std::size_t>(shandle, 2);
        } else {
            if (ritsuko::hdf5::exceeds_integer_limit(shandle, 64, false)) {
                throw std::runtime_error("expected a datatype that can fit into a 64-bit unsigned integer");
            }
            dims = load_dimensions_from_uint64_contents<std::size_t>(shandle, 2);
        }
    } catch (...) {
        std::throw_with_nested(std::runtime_error("failed to validate 'shape'"));
    }

    hsize_t nnz;
    try {
        auto dhandle = group.openDataSet("data");
        auto dspace = dhandle.getSpace();
        if (dspace.getSimpleExtentNdims() != 1) {
            throw std::runtime_error("expected a 1-dimensional dataset");
        }
        dspace.getSimpleExtentDims(&nnz);

        if (version.lt(1, 1, 0)) {
            array_type = translate_type_0_99(dhandle.getTypeClass());
            if (is_boolean_0_99(dhandle)) {
                array_type = BOOLEAN;
            }
        } else {
            try {
                auto thandle = dhandle.openAttribute("type");
                auto type = read_scalar_string_attribute(thandle);
                array_type = translate_type_1_1(type);
                if (!options.details_only) {
                    check_type_1_1(dhandle, array_type);
                }
            } catch (...) {
                std::throw_with_nested(std::runtime_error("failed to validate the 'type' attribute"));
            }
        }

        if (!options.details_only) {
            if (array_type != INTEGER && array_type != BOOLEAN && array_type != FLOAT) {
                throw std::runtime_error("dataset should be integer, float or boolean");
            }
            validate_missing_placeholder(dhandle, version);
        }
    } catch (...) {
        std::throw_with_nested(std::runtime_error("failed to validate 'data'"));
    }

    if (!options.details_only) {
        bool csc = true;
        if (version.ge(1, 1, 0)) {
            try {
                auto bhandle = group.openDataSet("by_column");
                if (bhandle.getSpace().getSimpleExtentNdims() != 0) {
                    throw std::runtime_error("expected a scalar dataset");
                }
                if (ritsuko::hdf5::exceeds_integer_limit(bhandle, 8, true)) {
                    throw std::runtime_error("expected a datatype that can fit into an 8-bit signed integer");
                }
                std::int8_t val;
                bhandle.read(&val, H5::PredType::NATIVE_INT8);
                csc = (val != 0);
            } catch (...) {
                std::throw_with_nested(std::runtime_error("failed to validate 'by_column'"));
            }
        }

        const auto primary = (csc ? dims[1] : dims[0]);
        const auto secondary = (csc ? dims[0] : dims[1]);

        std::vector<std::uint64_t> indptrs;
        try {
            auto iphandle = group.openDataSet("indptr");
            auto ipspace = iphandle.getSpace();
            if (ipspace.getSimpleExtentNdims() != 1) {
                throw std::runtime_error("expected a 1-dimensional dataset");
            }
            hsize_t iplen;
            ipspace.getSimpleExtentDims(&iplen);

            if (iplen == 0 || !sanisizer::is_equal(iplen - 1, primary)) { // avoid risk of potential overflow with primary + 1.
                throw std::runtime_error("dataset length should be equal to the number of " + (csc ? std::string("columns") : std::string("rows")) + " plus 1");
            }

            if (version.lt(1, 1, 0)) {
                if (iphandle.getTypeClass() != H5T_INTEGER) {
                    throw std::runtime_error("expected an integer dataset");
                }
                indptrs = load_non_negative_integer_vector_0_99<std::uint64_t>(iphandle, iplen);
            } else {
                if (ritsuko::hdf5::exceeds_integer_limit(iphandle, 64, false)) {
                    throw std::runtime_error("expected a datatype that can fit into a 64-bit unsigned integer");
                }
                sanisizer::resize(indptrs, iplen);
                iphandle.read(indptrs.data(), H5::PredType::NATIVE_UINT64);
            }

            iphandle.read(indptrs.data(), H5::PredType::NATIVE_UINT64);
            if (indptrs[0] != 0) {
                throw std::runtime_error("first entry should be 0");
            }
            if (!sanisizer::is_equal(indptrs.back(), nnz)) {
                throw std::runtime_error("last entry should be equal to the length of 'data'");
            }

            for (std::size_t p = 0; p < primary; ++p) {
                const auto start = indptrs[p];
                const auto end = indptrs[p + 1];
                if (start > end) {
                    throw std::runtime_error("entries must be sorted in increasing order");
                }
            }
        } catch (...) {
            std::throw_with_nested(std::runtime_error("failed to validate 'indptr'"));
        }

        try {
            auto ihandle = group.openDataSet("indices");
            auto ispace = ihandle.getSpace();
            if (ispace.getSimpleExtentNdims() != 1) {
                throw std::runtime_error("expected a 1-dimensional dataset");
            }
            hsize_t inum;
            ispace.getSimpleExtentDims(&inum);
            if (nnz != inum) {
                throw std::runtime_error("dataset length should be the same as 'data'"); 
            }

            if (version.lt(1, 1, 0)) {
                if (!ritsuko::hdf5::exceeds_integer_limit(ihandle, 64, true)) {
                    validate_sparse_indices<std::int64_t>(ihandle, indptrs, primary, secondary, csc, options.contiguous_chunk_size);
                } else if (!ritsuko::hdf5::exceeds_integer_limit(ihandle, 64, false)) {
                    validate_sparse_indices<std::uint64_t>(ihandle, indptrs, primary, secondary, csc, options.contiguous_chunk_size);
                } else {
                    throw std::runtime_error("expected an integer dataset");
                }

            } else {
                if (ritsuko::hdf5::exceeds_integer_limit(ihandle, 64, false)) {
                    throw std::runtime_error("expected a datatype that can fit into a 64-bit unsigned integer");
                }
                validate_sparse_indices<std::uint64_t>(ihandle, indptrs, primary, secondary, csc, options.contiguous_chunk_size);
            }
        } catch (...) {
            std::throw_with_nested(std::runtime_error("failed to validate 'indices'"));
        }

        if (group.exists("dimnames")) {
            try {
                auto dnhandle = group.openGroup("dimnames");
                validate_dimnames_internal(dnhandle, dims, version, options.contiguous_chunk_size);
            } catch (...) {
                std::throw_with_nested(std::runtime_error("failed to validate 'dimnames'"));
            }
        }
    }

    return ArrayDetails(array_type, std::move(dims));
}

}

#endif
