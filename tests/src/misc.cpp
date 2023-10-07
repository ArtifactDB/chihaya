#include <gtest/gtest.h>
#include "chihaya/chihaya.hpp"
#include "utils.h"

static void expect_version_error(std::string version, std::string msg) {
    expect_error([&]() { chihaya::parse_version_string(version); }, msg);
}

TEST(VersionParsing, Checks) {
    auto out = chihaya::parse_version_string("1.0.0");
    EXPECT_EQ(out.major, 1);
    EXPECT_EQ(out.minor, 0);
    EXPECT_EQ(out.patch, 0);

    out = chihaya::parse_version_string("123.45.6");
    EXPECT_EQ(out.major, 123);
    EXPECT_EQ(out.minor, 45);
    EXPECT_EQ(out.patch, 6);

    out = chihaya::parse_version_string("0.99"); // back-compatibiltiy.
    EXPECT_EQ(out.major, 0);
    EXPECT_EQ(out.minor, 99);
    EXPECT_EQ(out.patch, 0);
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
