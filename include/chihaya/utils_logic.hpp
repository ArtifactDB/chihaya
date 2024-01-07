#ifndef CHIHAYA_UTILS_LOGIC_HPP
#define CHIHAYA_UTILS_LOGIC_HPP

#include "H5Cpp.h"
#include "ritsuko/hdf5/hdf5.hpp"
#include "ritsuko/ritsuko.hpp"
#include <stdexcept>
#include "utils.hpp"

namespace chihaya {

ArrayDetails validate(const H5::Group&, const ritsuko::Version&);

namespace internal_logic {

inline bool is_valid_operation(const std::string& method) {
    return method == "&&" || method == "||";
}

inline ArrayDetails fetch_seed(const H5::Group& handle, const std::string& target, const ritsuko::Version& version) {
    const auto& ghandle = ritsuko::hdf5::open_group(handle, target.c_str());

    ArrayDetails output;
    try {
        output = ::chihaya::validate(ghandle, version);
    } catch (std::exception& e) {
        throw std::runtime_error("failed to validate '" + target + "'; " + std::string(e.what()));
    }

    if (output.type == STRING) {
        throw std::runtime_error("type of '" + target + "' should be integer, float or boolean");
    }
    return output;
}

}

}

#endif
