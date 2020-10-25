#include <catch2/catch.hpp>
#include "csv.hh"

TEST_CASE("parse_header", "[parse_header]") {
    auto header = csv::parse_header("colA,colB,colC\n");
    models::header_row expected{{"colA"}, {"colB"}, {"colC"}};

    REQUIRE(header == expected);
}

TEST_CASE("parse_body", "[parse_body]") {
    models::raw_chunk rc = {0, R"(a,b,c
d,e,f
g,h,i
)"};

    std::string parsed;
    csv::parse_body(std::move(rc), [&](models::row& row){
	REQUIRE(row.size() == 3);
	for(auto field : row) {
	    parsed += std::get<std::string>(field);
	}
    });

    REQUIRE(parsed == "abcdefghi");
}
