#include <catch2/catch.hpp>
#include "models.hh"

TEST_CASE("to_num", "[to_num]") {
    models::value a = "25";
    models::to_num(a);
    REQUIRE(std::get<double>(a) == 25.0);

    bool conv_failure = false;
    a = "hello";
    try {
	models::to_num(a);
    } catch (const std::invalid_argument &) {
	conv_failure = true;
    }
    REQUIRE(conv_failure);
}

TEST_CASE("to_str", "[to_str]") {
    models::value a = 50.0;
    models::to_str(a);
    REQUIRE(std::get<std::string>(a) == "50");
}

TEST_CASE("append_to_string", "[append_to_string]") {
    std::string string;
    models::row row = models::row{"hello", "50", "", "zzz"};
    models::append_to_string(string, row);

    REQUIRE(string == "hello,50,,zzz\n");
}
