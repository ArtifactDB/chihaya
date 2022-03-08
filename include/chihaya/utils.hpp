#ifndef CHIHAYA_UTILS_HPP
#define CHIHAYA_UTILS_HPP

#include <vector>
#include <string>

#include "H5Cpp.h"

namespace chihaya {

inline std::string load_string_attribute(const H5::Attribute& attr, const std::string field) {
    if (attr.getTypeClass() != H5T_STRING || attr.getSpace().getSimpleExtentNdims() != 0) {
        throw std::runtime_error(std::string("'") + field + "' attribute should be a scalar string");
    }

    H5::StrType stype(0, H5T_VARIABLE);
    H5::DataSpace sspace(H5S_SCALAR);
    std::string output;
    attr.read(stype, output);

    return output;
}

inline std::string load_string_attribute(const H5::Group& handle, const std::string& field, const std::string& extra) {
    if (!handle.attrExists(field)) {
        throw std::runtime_error(std::string("expected a '") + field + "' attribute" + extra);
    }
    return load_string_attribute(handle.openAttribute(field), field);
}

}

#endif
