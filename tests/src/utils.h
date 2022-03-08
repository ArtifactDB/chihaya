#ifndef UTILS_H
#define UTILS_H

inline H5::Group super_group_opener(const H5::Group& parent, const std::string& name, const std::map<std::string, std::string>& attributes) {
    auto ghandle = parent.createGroup(name);
    for (const auto& p : attributes) {
        H5::StrType stype(0, H5T_VARIABLE);
        auto ahandle = ghandle.createAttribute(p.first, stype, H5S_SCALAR);
        ahandle.write(stype, p.second);
    }
    return ghandle;
}

inline H5::Group operation_opener(const H5::Group& parent, const std::string& name, const std::string& operation) {
    std::map<std::string, std::string> attrs;
    attrs["delayed_type"] = "operation";
    attrs["delayed_operation"] = operation;
    return super_group_opener(parent, name, attrs);
}

inline H5::Group array_opener(const H5::Group& parent, const std::string& name, const std::string& array) {
    std::map<std::string, std::string> attrs;
    attrs["delayed_type"] = "array";
    attrs["delayed_array"] = array;
    return super_group_opener(parent, name, attrs);
}

inline H5::Group external_array_opener(const H5::Group& parent, const std::string& name, const std::vector<int>& dimensions, std::string type = "FLOAT") {
    auto ghandle = array_opener(parent, name, "external array");

    hsize_t ndim = dimensions.size();
    H5::DataSpace dspace(1, &ndim);
    auto dhandle = ghandle.createDataSet("dimensions", H5::PredType::NATIVE_INT, dspace);
    dhandle.write(dimensions.data(), H5::PredType::NATIVE_INT);

    H5::StrType stype(0, H5T_VARIABLE);
    auto thandle = ghandle.createDataSet("type", stype, H5S_SCALAR);
    thandle.write(type, stype, H5S_SCALAR);

    return ghandle;
}

inline H5::Group list_opener(const H5::Group& parent, const std::string& name, int length) {
    std::map<std::string, std::string> attrs;
    attrs["delayed_type"] = "list";
    auto ghandle = super_group_opener(parent, name, attrs);

    auto ahandle = ghandle.createAttribute("delayed_length", H5::PredType::NATIVE_INT, H5S_SCALAR);
    ahandle.write(H5::PredType::NATIVE_INT, &length);
    return ghandle;
}

template<typename T>
void add_vector(const H5::Group& handle, const std::string& name, const std::vector<T>& values) {
    hsize_t n = values.size();
    H5::DataSpace dspace(1, &n);
    if constexpr(std::is_same<T, int>::value) {
        auto dhandle = handle.createDataSet(name, H5::PredType::NATIVE_INT, dspace); 
        dhandle.write(values.data(), H5::PredType::NATIVE_INT);
    } else if constexpr(std::is_same<T, double>::value) {
        auto dhandle = handle.createDataSet(name, H5::PredType::NATIVE_DOUBLE, dspace); 
        dhandle.write(values.data(), H5::PredType::NATIVE_DOUBLE);
    } else {
        static_assert("vector type should be either an 'int' or 'double'"); 
    }
}

template<typename T>
void add_scalar(const H5::Group& handle, const std::string& name, T value) {
    H5::DataSpace dspace;
    if constexpr(std::is_same<T, int>::value) {
        auto dhandle = handle.createDataSet(name, H5::PredType::NATIVE_INT, dspace); 
        dhandle.write(&value, H5::PredType::NATIVE_INT);
    } else if constexpr(std::is_same<T, double>::value) {
        auto dhandle = handle.createDataSet(name, H5::PredType::NATIVE_DOUBLE, dspace); 
        dhandle.write(&value, H5::PredType::NATIVE_DOUBLE);
    } else {
        static_assert("vector type should be either an 'int' or 'double'"); 
    }
}

template<class Function>
void expect_error(Function op, std::string message) {
    EXPECT_ANY_THROW({
        try {
            op();
            std::cout << message << std::endl;
        } catch (std::exception& e) {
            std::string msg(e.what());
            EXPECT_TRUE(msg.find(message) != std::string::npos);
            throw;
        }
    });
}

#endif
