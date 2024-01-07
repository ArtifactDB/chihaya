#ifndef CHIHAYA_UTILS_MISC_HPP
#define CHIHAYA_UTILS_MISC_HPP

#include "H5Cpp.h"
#include "ritsuko/hdf5/hdf5.hpp"
#include "ritsuko/ritsuko.hpp"

namespace chihaya {

namespace internal_misc {

inline bool is_version_at_or_below(const ritsuko::Version& v, int major, int minor) {
    return v.major < major || (v.major == major && v.minor <= minor);
}

template<class V>
bool are_dimensions_equal(const V& left, const V& right) {
    if (left.size() != right.size()) {
        return false;
    } else {
        for (size_t i = 0; i < left.size(); ++i) {
            if (left[i] != right[i]) {
                return false;
            }
        }
    }
    return true;
}

inline void validate_missing_placeholder(const H5::DataSet& handle, const ritsuko::Version& version) {
    if (version.major == 0) {
        return;
    }

    const char* placeholder = "missing_placeholder";
    if (!handle.attrExists(placeholder)) {
        return;
    }

    auto ahandle = handle.openAttribute(placeholder);
    ritsuko::hdf5::check_missing_placeholder_attribute(handle, ahandle, /* type_class_only = */ (version.major == 1 && version.minor == 0));
}

}

}

#endif
