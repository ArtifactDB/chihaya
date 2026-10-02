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

[[noreturn]]
inline void wrap_error(const std::exception_ptr& err, const std::string& context) {
    try {
        std::rethrow_exception(err);
    } catch (std::exception& e) {
        throw std::runtime_error(context + "; " + std::string(e.what()));
    } catch (H5::Exception& e) {
        throw std::runtime_error(context + "; " + e.getDetailMsg());
    }
}

inline void validate_missing_placeholder(const H5::DataSet& handle, const ritsuko::Version& version) {
    if (version.lt(1, 0, 0)) {
        return;
    }

    const char* placeholder = "missing_placeholder";
    if (!handle.attrExists(placeholder)) {
        return;
    }

    auto ahandle = handle.openAttribute(placeholder);
    if (ahandle.getSpace().getSimpleExtentNdims() != 0) {
        throw std::runtime_error("expected the '" + std::string(placeholder) + "' attribute to be a scalar");
    }

    if (handle.getTypeClass() == H5T_STRING) {
        if (ahandle.getTypeClass() != H5T_STRING) {
            throw std::runtime_error("expected the '" + std::string(placeholder) + "' attribute to use a string datatype class");
        }
        try {
            ritsuko::hdf5::validate_scalar_string(ahandle);
        } catch (...) {
            wrap_error(std::current_exception(), "failed to validate the '" + std::string(placeholder) + "' attribute");
        }
    } else {
        if (version.lt(1, 1, 0)) {
            // Older versions only required the same type class.
            if (ahandle.getTypeClass() != handle.getTypeClass()) {
                throw std::runtime_error("expected the '" + std::string(placeholder) + "' attribute to use the same datatype class as its dataset");
            }
        } else {
            if (ahandle.getDataType() != handle.getDataType()) {
                throw std::runtime_error("expected the '" + std::string(placeholder) + "' attribute to use the same datatype as its dataset");
            }
        }
    }
}

inline ArrayDetails fetch_seed(const H5::Group& handle, const std::string& name, const ritsuko::Version& version, const Options& options) {
    auto shandle = handle.openGroup(name);
    try {
        return validate(shandle, version, options);
    } catch (...) {
        wrap_error(std::current_exception(), "failed to validate '" + name + "'");
    }
}

inline H5::DataSet safe_open_scalar_string_dataset(const H5::Group& handle, const std::string& name) {
    auto dhandle = handle.openDataSet(name);
    if (dhandle.getSpace().getSimpleExtentNdims() != 0) {
        throw std::runtime_error("'" + name + "' dataset should be scalar");
    }
    if (!ritsuko::hdf5::is_utf8_string(dhandle)) {
        throw std::runtime_error("'" + name + "' dataset should use a datatype that can be represented by a UTF-8 encoded string");
    }
    return dhandle;
}

inline std::string load_scalar_string_dataset(const H5::Group& handle, const std::string& name) {
    auto dhandle = safe_open_scalar_string_dataset(handle, name);
    try {
        return ritsuko::hdf5::read_scalar_string(dhandle);
    } catch (...) {
        wrap_error(std::current_exception(), "failed to read the '" + name + "' dataset");
    }
}

template<typename Handle_>
H5::Attribute safe_open_scalar_string_attribute(const Handle_& handle, const std::string& name) {
    auto ahandle = handle.openAttribute(name);
    if (ahandle.getSpace().getSimpleExtentNdims() != 0) {
        throw std::runtime_error("'" + name + "' attribute should be scalar");
    }
    if (!ritsuko::hdf5::is_utf8_string(ahandle)) {
        throw std::runtime_error("'" + name + "' attribute should use a datatype that can be represented by a UTF-8 encoded string");
    }
    return ahandle;
}

template<typename Handle_>
std::string load_scalar_string_attribute(const Handle_& handle, const std::string& name) {
    auto ahandle = safe_open_scalar_string_attribute(handle, name);
    try {
        return ritsuko::hdf5::read_scalar_string(ahandle);
    } catch (...) {
        wrap_error(std::current_exception(), "failed to read the '" + name + "' attribute");
    }
}

}

#endif
