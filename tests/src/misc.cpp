#include <gtest/gtest.h>
#include "chihaya/chihaya.hpp"
#include "utils.h"

static void expect_version_error(std::string version, std::string msg) {
    expect_error([&]() { chihaya::parse_version_string(version); }, msg);
}

TEST(VersionParsing, Errors) {
    expect_version_error("", "empty");
    expect_version_error("0.1.1", "leading zeros");
    expect_version_error("a.1.1", "non-digit");
    expect_version_error("1", "minor version");
    expect_version_error("1.", "minor version");
    expect_version_error("1.01.1", "leading zeros");
    expect_version_error("1.a.1", "non-digit");
    expect_version_error("1.0", "patch version");
    expect_version_error("1.0.", "patch version");
    expect_version_error("1.0.00", "leading zeros");
    expect_version_error("1.0.a", "non-digit");
}
