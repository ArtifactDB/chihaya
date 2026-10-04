#ifndef CHIHAYA_UTILS_MISC_HPP
#define CHIHAYA_UTILS_MISC_HPP

#include "H5Cpp.h"

#include "ritsuko/ritsuko.hpp"
#include "sanisizer/sanisizer.hpp"

#include <string>
#include <stdexcept>
#include <type_traits>
#include <cstddef>
#include <cstdint>
#include <limits>

#include "utils_public.hpp"

namespace chihaya {

ArrayDetails validate(const H5::Group&, const ritsuko::Version&, const Options&);

template<typename Input_>
using I = std::remove_cv_t<std::remove_reference_t<Input_> >;

inline void validate_missing_placeholder(const H5::DataSet& handle, const ritsuko::Version& version) {
    if (version.lt(1, 0, 0)) {
        return;
    }

    const char* placeholder = "missing_placeholder";
    if (!handle.attrExists(placeholder)) {
        return;
    }

    try {
        auto ahandle = handle.openAttribute(placeholder);
        if (ahandle.getSpace().getSimpleExtentNdims() != 0) {
            throw std::runtime_error("expected a scalar attribute");
        }

        if (handle.getTypeClass() == H5T_STRING) {
            if (ahandle.getTypeClass() != H5T_STRING) {
                throw std::runtime_error("expected a string datatype");
            }
            ritsuko::hdf5::validate_scalar_string(ahandle);
        } else {
            if (version.lt(1, 1, 0)) {
                // Older versions only required the same type class.
                if (ahandle.getTypeClass() != handle.getTypeClass()) {
                    throw std::runtime_error("expected the same datatype class as its parent dataset");
                }
            } else {
                if (ahandle.getDataType() != handle.getDataType()) {
                    throw std::runtime_error("expected the same datatype as its parent dataset");
                }
            }
        }
    } catch (...) {
        std::throw_with_nested(std::runtime_error("failed to validate the '" + std::string(placeholder) + "' attribute"));
    }
}

inline void validate_scalar_string_dataset(const H5::DataSet& dhandle) {
    if (dhandle.getSpace().getSimpleExtentNdims() != 0) {
        throw std::runtime_error("expected a scalar dataset");
    }
    if (!ritsuko::hdf5::is_utf8_string(dhandle)) {
        throw std::runtime_error("expected a datatype that can be represented by a UTF-8 encoded string");
    }
}

inline std::string read_scalar_string_dataset(const H5::DataSet& dhandle) {
    validate_scalar_string_dataset(dhandle);
    return ritsuko::hdf5::read_scalar_string(dhandle);
}

inline void validate_scalar_string_attribute(const H5::Attribute& ahandle) {
    if (ahandle.getSpace().getSimpleExtentNdims() != 0) {
        throw std::runtime_error("expected a scalar attribute");
    }
    if (!ritsuko::hdf5::is_utf8_string(ahandle)) {
        throw std::runtime_error("expected a datatype that can be represented by a UTF-8 encoded string");
    }
}

inline std::string read_scalar_string_attribute(const H5::Attribute& ahandle) {
    validate_scalar_string_attribute(ahandle);
    return ritsuko::hdf5::read_scalar_string(ahandle);
}

}

#endif
