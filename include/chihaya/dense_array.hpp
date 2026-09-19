#ifndef CHIHAYA_DENSE_ARRAY_HPP
#define CHIHAYA_DENSE_ARRAY_HPP

#include "H5Cpp.h"
#include "ritsuko/ritsuko.hpp"
#include "sanisizer/sanisizer.hpp"

#include <vector>
#include <cstdint>
#include <stdexcept>
#include <cstddef>

#include "utils_public.hpp"
#include "utils_misc.hpp"
#include "utils_type.hpp"
#include "utils_dimensions.hpp"

/**
 * @file dense_array.hpp
 * @brief Dense array, stored inside the file.
 */

namespace chihaya {

/**
 * @cond
 */
template<typename Output_>
void transplant_dimensions(std::vector<hsize_t>& src, std::vector<Output_>& output) {
    if constexpr(std::is_same<hsize_t, Output_>::value) {
        // Avoid a copy if we can.
        output = std::move(src);
    } else {
        const auto ndims = src.size();
        sanisizer::resize(output, ndims);
        for (I<decltype(ndims)> d = 0; d < ndims; ++d) {
            output[d] = sanisizer::cast<Output_>(src[d]);
        }
    }
}
/**
 * @endcond
 */

/**
 * @param handle An open handle on a HDF5 group representing a dense array.
 * @param version Version of the **chihaya** specification.
 * @param options Validation options.
 *
 * @return Details of the dense array.
 * Otherwise, if the validation failed, an error is raised.
 */
inline ArrayDetails validate_dense_array(const H5::Group& handle, const ritsuko::Version& version, [[maybe_unused]] Options& options) {
    ArrayDetails output;

    {
        auto dhandle = handle.openDataSet("data");
        auto dspace = dhandle.getSpace();
        const auto ndims = dspace.getSimpleExtentNdims();
        if (ndims == 0) {
            throw std::runtime_error("'data' should have non-zero dimensions");
        }
        auto dims = sanisizer::create<std::vector<hsize_t> >(ndims);
        dspace.getSimpleExtentDims(dims.data());

        try {
            if (version.lt(1, 1, 0)) {
                output.type = translate_type_0_99(dhandle.getTypeClass());
                if (is_boolean_0_99(dhandle)) {
                    output.type = BOOLEAN;
                }
            } else {
                auto type = load_scalar_string_attribute(dhandle, "type");
                output.type = translate_type_1_1(type);
                if (!options.details_only) {
                    check_type_1_1(dhandle, output.type);
                }
            }

            if (!options.details_only) {
                validate_missing_placeholder(dhandle, version);
                if (dhandle.getTypeClass() == H5T_STRING) {
                    ritsuko::hdf5::validate_nd_strings(dhandle, dims);
                }
            }

        } catch (std::exception& e) {
            throw std::runtime_error("failed to validate 'data'; " + std::string(e.what()));
        }

        transplant_dimensions(dims, output.dimensions);
    }

    bool native;
    {
        auto nhandle = handle.openDataSet("native");
        if (nhandle.getSpace().getSimpleExtentNdims() != 0) {
            throw std::runtime_error("'native' attribute should be a scalar");
        }

        if (version.lt(1, 1, 0)) {
            native = load_boolean_scalar_0_99(nhandle);
        } else {
            if (ritsuko::hdf5::exceeds_integer_limit(nhandle, 8, true)) {
                throw std::runtime_error("'native' attribute should use a datatype that fits into an 8-bit signed integer");
            }
            std::int8_t tmp_native;
            nhandle.read(&tmp_native, H5::PredType::NATIVE_INT);
            native = tmp_native;
        }
    }

    // Do this before applying the 'native' reversal.
    if (!options.details_only) {
        if (handle.exists("dimnames")) {
            validate_dimnames_internal(handle, output.dimensions, version);
        }
    }

    if (!native) {
        std::reverse(output.dimensions.begin(), output.dimensions.end());
    }

    return output;
}

}

#endif
