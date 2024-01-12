#include "utils.h"

chihaya::ArrayDetails test_validate(const std::string& path, const std::string& name) {
    return chihaya::validate(path, name);
}

TEST(Validate, CustomRegistry) {
    const char* path = "Test_validate.h5";
    chihaya::State state;

    std::vector<std::string> known_arrays;
    state.array_validate_registry["constant array"] = [&](const H5::Group& h, const ritsuko::Version& v) -> chihaya::ArrayDetails {
        known_arrays.push_back("constant array"); 
        auto it = chihaya::array_validate_registry.find("constant array");
        return (it->second)(h, v);
    }; 

    std::vector<std::string> known_operations;
    state.operation_validate_registry["transpose"] = [&](const H5::Group& h, const ritsuko::Version& v, chihaya::State& s) -> chihaya::ArrayDetails { 
        known_operations.push_back("transpose"); 
        auto it = chihaya::operation_validate_registry.find("transpose");
        return (it->second)(h, v, s);
    }; 

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = operation_opener(fhandle, "WHEE", "transpose");
        add_version_string(ghandle, 1100000);
        add_numeric_vector<int>(ghandle, "permutation", { 1, 0 }, H5::PredType::NATIVE_UINT32);

        auto shandle = array_opener(ghandle, "seed", "constant array");
        add_numeric_vector<int>(shandle, "dimensions", { 20, 17 }, H5::PredType::NATIVE_UINT32);
        auto dhandle = add_numeric_scalar(shandle, "value", 1, H5::PredType::NATIVE_INT32);
        add_string_attribute(dhandle, "type", "INTEGER");
    }

    chihaya::validate(path, "WHEE", state);
    EXPECT_EQ(known_arrays.size(), 1);
    EXPECT_EQ(known_arrays.front(), "constant array");
    EXPECT_EQ(known_operations.size(), 1);
    EXPECT_EQ(known_operations.front(), "transpose");
}
