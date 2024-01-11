#ifndef CHIHAYA_UTILS_LOGIC_HPP
#define CHIHAYA_UTILS_LOGIC_HPP

#include "H5Cpp.h"
#include "ritsuko/hdf5/hdf5.hpp"
#include "ritsuko/ritsuko.hpp"

#include <stdexcept>

#include "utils_misc.hpp"

namespace chihaya {

ArrayDetails validate(const H5::Group&, const ritsuko::Version&);

namespace internal_logic {

inline bool is_valid_operation(const std::string& method) {
    return method == "&&" || method == "||";
}

inline ArrayDetails fetch_seed(const H5::Group& handle, const std::string& target, const ritsuko::Version& version) {
    auto output = internal_misc::load_seed_details(handle, target, version);
    if (output.type == STRING) {
        throw std::runtime_error("type of '" + target + "' should be integer, float or boolean");
    }
    return output;
}

}

}

#endif
