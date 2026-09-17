#ifndef CHIHAYA_UTILS_LIST_HPP
#define CHIHAYA_UTILS_LIST_HPP

#include <map>
#include <string>
#include <stdexcept>
#include <limits>

#include "H5Cpp.h"
#include "ritsuko/ritsuko.hpp"
#include "sanisizer/sanisizer.hpp"

#include "utils_misc.hpp"
#include "utils_type.hpp"

namespace chihaya {

struct ListDetails {
    std::size_t length;
    std::map<std::size_t, std::string> present;
};

inline ListDetails validate_list(const H5::Group& handle, const ritsuko::Version& version) {
    ListDetails output;

    if (version.lt(1, 1, 0)) {
        auto ahandle = handle.openAttribute("delayed_type");
        if (!ritsuko::hdf5::is_utf8_string(ahandle)) {
            throw std::runtime_error("expected 'delayed_type' attribute to use a string datatype");
        }
        if (ahandle.getSpace().getSimpleExtentNdims() != 0) {
            throw std::runtime_error("expected 'delayed_type' attribute to be scalar");
        }
        auto dtype = ritsuko::hdf5::read_scalar_string(ahandle);
        if (dtype != "list") {
            throw std::runtime_error("expected 'delayed_type = \"list\"' for a list");
        }
    }

    const char* old_name = "delayed_length";
    const char* new_name = "length";
    const char* actual_name = (version.lt(1, 1, 0) ? old_name : new_name);
    {
        auto lhandle = handle.openAttribute(actual_name);
        if (lhandle.getSpace().getSimpleExtentNdims() != 0) {
            throw std::runtime_error("expected the '" + std::string(actual_name) + "' attribute to be a scalar");
        } 

        if (version.lt(1, 1, 0)) {
            output.length = load_non_negative_integer_scalar_0_99<std::size_t>(lhandle);
        } else {
            if (ritsuko::hdf5::exceeds_integer_limit(lhandle, 64, false)) {
                throw std::runtime_error("datatype of the '" + std::string(actual_name) + "' attribute should fit inside a 64-bit unsigned integer");
            }
            std::uint64_t l;
            lhandle.read(H5::PredType::NATIVE_UINT64, &l);
            output.length = sanisizer::cast<std::size_t>(l);
        }
    }

    const auto nobj = handle.getNumObjs();
    if (sanisizer::is_greater_than(nobj, output.length)) {
        throw std::runtime_error("more objects in the list than are specified by '" + std::string(actual_name) + "'");
    }
    for (I<decltype(nobj)> i = 0; i < nobj; ++i) {
        std::string name = handle.getObjnameByIdx(i);

        // Aaron's cheap and dirty atoi!
        std::size_t sofar = 0;
        for (auto c : name) {
            if (c < '0' || c > '9') {
                throw std::runtime_error("'" + name + "' is not a valid name for a list index");
            }
            sofar *= 10;
            sofar += (c - '0'); 
        }

        if (sofar >= output.length) {
            throw std::runtime_error("'" + name + "' is out of range for a list"); 
        }
        output.present[sofar] = name;
    }

    return output;
}

}

#endif
