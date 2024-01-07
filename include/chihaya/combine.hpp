#ifndef CHIHAYA_COMBINE_HPP
#define CHIHAYA_COMBINE_HPP

#include "H5Cpp.h"
#include "ritsuko/hdf5/hdf5.hpp"

#include <stdexcept>
#include <vector>
#include <string>
#include <cstdint>

#include "utils_list.hpp"
#include "utils_misc.hpp"

/**
 * @file combine.hpp
 * @brief Validation for delayed combining operations.
 */

namespace chihaya {

/**
 * @cond
 */
inline ArrayDetails validate(const H5::Group&, const Version&);
/**
 * @endcond
 */

/**
 * @namespace chihaya::combine
 * @brief Namespace for delayed combining operations.
 */
namespace combine {

/**
 * @param handle An open handle on a HDF5 group representing a combining operation.
 * @param version Version of the **chihaya** specification.
 * 
 * @return Details of the combined object.
 * Otherwise, if the validation failed, an error is raised.
 */
inline ArrayDetails validate(const H5::Group& handle, const Version& version) {
    uint64_t along = 0;
    {
        auto ahandle = ritsuko::hdf5::open_dataset(handle, "along");
        if (ahandle.getSpace().getSimpleExtentNdims() != 0) {
            throw std::runtime_error("'along' should be an integer scalar");
        }

        if (internal_misc::is_version_at_or_below(version.major, 1, 0)) {
            if (ahandle.getTypeClass() != H5T_INTEGER) {
                throw std::runtime_error("datatype for 'along' should be integer");
            }
            int along_tmp;
            ahandle.read(&along_tmp, H5::PredType::NATIVE_INT);
            if (along_tmp < 0) {
                throw std::runtime_error("'along' should contain a non-negative value");
            }
            along = along_tmp;

        } else {
            if (ritsuko::hdf5::exceeds_integer_limit(ahandle, 64, false)) {
                throw std::runtime_error("datatype for 'along' should fit in a 64-bit unsigned integer");
            }
            ahandle.read(&along, H5::PredType::NATIVE_UINT64);
        }
    }

    internal_list::ListDetails list_params;
    {
        auto shandle = ritsuko::hdf5::open_group(handle, "seeds");
        try {
            list_params = internal_list::validate(shandle, version);
        } catch (std::exception& e) {
            throw std::runtime_error(std::string("failed to load 'seeds' list; ") + e.what());
        }
        if (list_params.present.size() != list_params.length) {
            throw std::runtime_error("missing elements in the 'seeds' list");
        }
    }

    std::vector<size_t> dimensions;
    ArrayType type = BOOLEAN;
    {
        bool first = true;

        for (const auto& p : list_params.present) {
            auto current = ritsuko::hdf5::open_group(shandle, p.second);

            ArrayDetails cur_seed;
            try {
                cur_seed = ::chihaya::validate(current, version);
            } catch (std::exception& e) {
                throw std::runtime_error("failed to validate 'seeds/" + p.second + "'; " + std::string(e.what()));
            }

            if (first) {
                type = cur_seed.type;
                dimensions = cur_seed.dimensions;
                if (static_cast<size_t>(along) >= dimensions.size()) {
                    throw std::runtime_error("'along' should be less than the seed dimensionality");
                }
                first = false;
            } else {
                if (type < cur_seed.type) {
                    type = cur_seed.type;
                }
                if (dimensions.size() != cur_seed.dimensions.size()) {
                    throw std::runtime_error("dimensionality mismatch between seeds");
                }
                for (size_t d = 0; d < dimensions.size(); ++d) {
                    if (d == static_cast<size_t>(along)) {
                        dimensions[d] += cur_seed.dimensions[d];
                    } else if (dimensions[d] != cur_seed.dimensions[d]) {
                        throw std::runtime_error("inconsistent dimension extents between seeds");
                    }
                }
            }
        }
    }

    return ArrayDetails(type, std::move(dimensions));
}

}

#endif
