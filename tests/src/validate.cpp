#include "utils.h"

chihaya::ArrayDetails test_validate(const std::string& path, const std::string& name) {
    return chihaya::validate(path, name);
}

chihaya::ArrayDetails test_validate_skip(const std::string& path, const std::string& name) {
    chihaya::Options opts;
    opts.details_only = true;
    return chihaya::validate(path, name, opts);
}

TEST(Validate, CustomRegistry) {
    const char* path = "Test_validate.h5";
    chihaya::Options options;

    std::vector<std::string> known_arrays;
    options.array_validate_registry["constant array"] = [&](const H5::Group& h, const ritsuko::Version& v, chihaya::Options& o) -> chihaya::ArrayDetails {
        known_arrays.push_back("constant array"); 
        return chihaya::constant_array::validate(h, v, o);
    }; 

    std::vector<std::string> known_operations;
    options.operation_validate_registry["transpose"] = [&](const H5::Group& h, const ritsuko::Version& v, chihaya::Options& o) -> chihaya::ArrayDetails { 
        known_operations.push_back("transpose"); 
        return chihaya::transpose::validate(h, v, o);
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

    chihaya::validate(path, "WHEE", options);
    EXPECT_EQ(known_arrays.size(), 1);
    EXPECT_EQ(known_arrays.front(), "constant array");
    EXPECT_EQ(known_operations.size(), 1);
    EXPECT_EQ(known_operations.front(), "transpose");

    options.array_validate_registry["constant array"] = [&](const H5::Group&, const ritsuko::Version&, chihaya::Options&) -> chihaya::ArrayDetails {
        throw std::runtime_error("uh no");
    };
    expect_error([&]() { chihaya::validate(path, "WHEE", options); }, "uh no");

    options.operation_validate_registry["transpose"] = [&](const H5::Group&, const ritsuko::Version&, chihaya::Options&) -> chihaya::ArrayDetails {
        throw std::runtime_error("no means no!");
    };
    expect_error([&]() { chihaya::validate(path, "WHEE", options); }, "no means no!");
}

TEST(Validate, Errors) {
    const char* path = "Test_validate.h5";

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = operation_opener(fhandle, "WHEE", "FOO");
        add_version_string(ghandle, 1100000);
    }
    expect_error(path, "WHEE", "unknown operation type 'FOO'");

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = array_opener(fhandle, "seed", "BAR");
        add_version_string(ghandle, 1100000);
    }
    expect_error(path, "seed", "unknown array type 'BAR'");

    {
        H5::H5File fhandle(path, H5F_ACC_TRUNC);
        auto ghandle = fhandle.createGroup("FOO");
        add_string_attribute(ghandle, "delayed_type", "YAY");
    }
    expect_error(path, "seed", "unknown object type 'YAY'");
}
