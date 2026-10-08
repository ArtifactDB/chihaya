#ifndef CHIHAYA_UTILS_LIST_HPP
#define CHIHAYA_UTILS_LIST_HPP

#include <vector>
#include <string>
#include <stdexcept>
#include <exception>
#include <limits>

#include "H5Cpp.h"
#include "ritsuko/ritsuko.hpp"
#include "sanisizer/sanisizer.hpp"

#include "utils_misc.hpp"
#include "utils_type.hpp"

namespace chihaya {

struct ListDetails {
    std::size_t length;
    std::vector<std::pair<std::size_t, std::string> > present; // NOTE: not guaranteed to be sorted!
};

inline ListDetails validate_list(const H5::Group& handle, const ritsuko::Version& version) {
    ListDetails output;

    if (version.lt(1, 1, 0)) {
        std::string dtype;
        try {
            auto ahandle = handle.openAttribute("delayed_type");
            dtype = read_scalar_string_attribute(ahandle);
        } catch (...) {
            std::throw_with_nested(std::runtime_error("failed to validate the 'delayed_type' attribute"));
        }
        if (dtype != "list") {
            throw std::runtime_error("expected 'delayed_type = \"list\"' for a list");
        }
    }

    const char* old_name = "delayed_length";
    const char* new_name = "length";
    const char* actual_name = (version.lt(1, 1, 0) ? old_name : new_name);

    try {
        auto lhandle = handle.openAttribute(actual_name);
        if (lhandle.getSpace().getSimpleExtentNdims() != 0) {
            throw std::runtime_error("expected attribute to be a scalar");
        } 

        if (version.lt(1, 1, 0)) {
            output.length = load_non_negative_integer_scalar_0_99<std::size_t>(lhandle);
        } else {
            if (ritsuko::hdf5::exceeds_integer_limit(lhandle, 64, false)) {
                throw std::runtime_error("datatype should fit inside a 64-bit unsigned integer");
            }
            std::uint64_t l;
            lhandle.read(H5::PredType::NATIVE_UINT64, &l);
            output.length = sanisizer::cast<std::size_t>(l);
        }
    } catch (...) {
        std::throw_with_nested(std::runtime_error("failed to validate the '" + std::string(actual_name) + "' attribute"));
    }

    const auto nobj = handle.getNumObjs();
    if (sanisizer::is_greater_than(nobj, output.length)) {
        throw std::runtime_error("more objects in the list than are specified by '" + std::string(actual_name) + "'");
    }

    const std::size_t mult_limit = output.length / 10;
    const std::size_t add_limit = output.length % 10;

    for (I<decltype(nobj)> i = 0; i < nobj; ++i) {
        std::string name = handle.getObjnameByIdx(i);

        if (name.size() > 1 && name[0] == '0') {
            throw std::runtime_error("dataset name '" + name + "' should not contain leading zeros");
        }

        // Homebrewed atoi with in-built checks that the result doesn't exceed 'output.length'.
        // We do the checks inside the loop to additionally protect against overflow of size_t.
        std::size_t sofar = 0;
        for (auto c : name) {
            if (c < '0' || c > '9') {
                throw std::runtime_error("'" + name + "' is not a valid name for a list index");
            }
            const auto diff = c - '0';
            if (sofar > mult_limit || (sofar == mult_limit && sanisizer::is_greater_than_or_equal(diff, add_limit))) {
                throw std::runtime_error("dataset '" + name + "' is out of bounds for a list of length " + std::to_string(output.length));
            }
            sofar *= 10;
            sofar += diff;
        }

        output.present.emplace_back(sofar, name);
    }

    return output;
}

}

#endif
