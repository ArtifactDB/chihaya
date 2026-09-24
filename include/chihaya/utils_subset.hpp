#ifndef CHIHAYA_UTILS_SUBSET_HPP
#define CHIHAYA_UTILS_SUBSET_HPP

#include "H5Cpp.h"
#include "ritsuko/ritsuko.hpp"

#include <vector>
#include <stdexcept>
#include <cstdint>
#include <cstddef>

#include "utils_list.hpp"
#include "utils_misc.hpp"

namespace chihaya {

template<typename Index_>
void validate_subset_indices(const H5::DataSet& dhandle, hsize_t len, std::size_t extent, hsize_t contiguous_chunk_size) {
    ritsuko::hdf5::Stream1dNumericDataset<Index_> stream(
        &dhandle,
        len,
        [&]{
            ritsuko::hdf5::Stream1dNumericDatasetOptions opt;
            opt.contiguous_chunk_size = contiguous_chunk_size;
            return opt;
        }()
    );

    auto buffer = sanisizer::create<std::vector<Index_> >(stream.chunk_size());
    while (true) {
        const auto available = stream.load(buffer.data());
        if (available == 0) {
            break;
        }
        for (I<decltype(available)> i = 0; i < available; ++i) {
            const auto idx = buffer[i];
            if (idx < 0) {
                throw std::runtime_error("indices should be non-negative in '" + ritsuko::hdf5::get_name(dhandle) + "'");
            }
            if (sanisizer::is_greater_than_or_equal(idx, extent)) {
                throw std::runtime_error("indices out of range in '" + ritsuko::hdf5::get_name(dhandle) + "'");
            }
        }
    }
}

inline std::vector<std::pair<std::size_t, std::size_t> > validate_subset_index_list(
    const H5::Group& ihandle,
    const std::vector<std::size_t>& seed_dims,
    const ritsuko::Version& version,
    const hsize_t contiguous_chunk_size
) {
    ListDetails list_params;
    try {
        list_params = validate_list(ihandle, version);
    } catch (std::exception& e) {
        throw std::runtime_error("failed to load 'index' list; " + std::string(e.what()));
    }

    if (list_params.length != seed_dims.size()) {
        throw std::runtime_error("length of 'index' should be equal to number of dimensions in 'seed'");
    }

    std::vector<std::pair<std::size_t, std::size_t> > collected;

    for (const auto& p : list_params.present) {
        try {
            auto dhandle = ihandle.openDataSet(p.second);
            auto dspace = dhandle.getSpace();
            if (dspace.getSimpleExtentNdims() != 1) {
                throw std::runtime_error("expected a 1-dimensional dataset");
            }
            hsize_t len;
            dspace.getSimpleExtentDims(&len);

            if (version.lt(1, 1, 0)) {
                // Older versions didn't actually specify the integer type, so we just check we can load it into an 'int' of any type.
                if (!ritsuko::hdf5::exceeds_integer_limit(dhandle, 64, true)) {
                    validate_subset_indices<std::int64_t>(dhandle, len, seed_dims[p.first], contiguous_chunk_size);
                } else if (!ritsuko::hdf5::exceeds_integer_limit(dhandle, 64, false)) {
                    validate_subset_indices<std::uint64_t>(dhandle, len, seed_dims[p.first], contiguous_chunk_size);
                } else {
                    throw create_integer_error_0_99(ritsuko::hdf5::get_name(dhandle), dhandle.getTypeClass());
                }
            } else {
                if (ritsuko::hdf5::exceeds_integer_limit(dhandle, 64, false)) {
                    throw std::runtime_error("datatype should fit into a 64-bit unsigned integer");
                }
                validate_subset_indices<std::uint64_t>(dhandle, len, seed_dims[p.first], contiguous_chunk_size);
            }

            collected.emplace_back(p.first, len);
        } catch (std::exception& e) {
            throw std::runtime_error("failed to validate 'index/" + p.second + "'; " + std::string(e.what()));
        }
    }

    return collected;
}

}

#endif
