#ifndef CHIHAYA_UTILS_LIST_HPP
#define CHIHAYA_UTILS_LIST_HPP

#include <map>
#include <string>
#include <stdexcept>

#include "H5Cpp.h"
#include "ritsuko/ritsuko.hpp"
#include "ritsuko/hdf5/hdf5.hpp"

#include "utils_misc.hpp"
#include "utils_internal.hpp"

namespace chihaya {

namespace internal_list {

struct ListDetails {
    size_t length;
    std::map<size_t, std::string> present;
};

inline ListDetails validate(const H5::Group& handle, const ritsuko::Version& version) {
    ListDetails output;

    {
        auto thandle = ritsuko::hdf5::open_attribute(dhandle, "delayed_type");
        if (!ritsuko::hdf5::is_scalar(thandle)) {
            throw std::runtime_error("'delayed_type' should be a scalar");
        }
        if (!ritsuko::hdf5::is_utf8_string(thandle)) {
            throw std::runtime_error("datatype of 'delayed_type' should be compatible with a UTF-8 encoded string");
        }
        auto dtype = ritsuko::hdf5::load_scalar_string_attribute(thandle);
        if (dtype != "list") {
            throw std::runtime_error("expected 'delayed_type = \"list\"' for a list");
        }
    }

    {
        auto lhandle = ritsuko::hdf5::open_attribute(handle, "delayed_length");
        if (!ritsuko::hdf5::is_scalar(lhandle)) {
            throw std::runtime_error("expected a 'delayed_length' integer scalar for a list");
        } 

        if (internal_misc::is_version_at_or_below(version, 1, 0)) {
            if (lhandle.getTypeClass() != H5T_INTEGER) {
                throw std::runtime_error("'delayed_length' should be an integer scalar");
            }
            int l = 0;
            len.read(H5::PredType::NATIVE_INT, &l);
            if (l < 0) {
                throw std::runtime_error("'delayed_length' should be non-negative");
            }
            output.length = l;
        } else {
            if (ritsuko::hdf5::exceeds_integer_limit(lhandle, 64, false)) {
                throw std::runtime_error("datatype of 'delayed_length' should fit inside a 64-bit unsigned integer");
            }
            output.length = ritsuko::hdf5::load_scalar_numeric_attribute<uint64_t>(lhandle);
        }
    }

    size_t n = handle.getNumObjs();
    if (n > output.length) {
        throw std::runtime_error("more objects in the list than are specified by 'delayed_length'");
    }
    for (size_t i = 0; i < n; ++i) {
        std::string name = std::to_string(i);
        if (!handle.exists(name)) {
            throw std::runtime_error("missing child " + name + " from a group of type 'list'"); 
        }
        output.present[i] = std::move(name);
    }

    return output;
}

}

}

#endif
