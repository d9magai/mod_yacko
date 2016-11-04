#define CATCH_CONFIG_MAIN
#include "yacko/utils/stringutils.h"
#include "Catch/include/catch.hpp"

TEST_CASE("parseUri Test", "[utils][stringutils]") {

    std::map<std::string, std::string> map = Yacko::Utils::parseUri(std::string("/yacko/bucket/path/to/object"));
    REQUIRE(map["bucket"] == "bucket");
    REQUIRE(map["objectkey"] == "path/to/object");
}

TEST_CASE("parseArgs Test", "[utils][stringutils]") {

    std::map<std::string, std::string> map = Yacko::Utils::parseArgs(std::string("?a=b&cde=fgh&i=123"));
    CHECK(map["a"] == "b");
    CHECK(map["cde"] == "fgh");
    CHECK(map["i"] == "123");
}
