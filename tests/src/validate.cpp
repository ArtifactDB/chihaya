#include "utils.h"

chihaya::ArrayDetails test_validate(const std::string& path, const std::string& name) {
    return chihaya::validate(path, name);
}

TEST(Validate, Callbacks) {
    const char* path = "Test_validate.h5";

    std::vector<std::string> known_arrays;
    std::vector<std::string> known_operations;
    chihaya::Callbacks clbk;
    clbk.array = [&](const std::string& t, const H5::Group&, const ritsuko::Version&) { known_arrays.push_back(t); }; 
    clbk.operation = [&](const std::string& t, const H5::Group&, const ritsuko::Version&) { known_operations.push_back(t); }; 

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

    chihaya::validate(path, "WHEE", clbk);
    EXPECT_EQ(known_arrays.size(), 1);
    EXPECT_EQ(known_arrays.front(), "constant array");
    EXPECT_EQ(known_operations.size(), 1);
    EXPECT_EQ(known_operations.front(), "transpose");
}
