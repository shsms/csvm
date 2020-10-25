#include <catch2/catch.hpp>
#include "parser.hh"
#include "../csv/csv.hh"

TEST_CASE("parser", "[parser::run]") {
    cli_args args;
    engine::engine e(args);

    REQUIRE((e.string() == ""));

    bool parse_failure = false;
    try {
	parser::run("to_num(cc", e);
    } catch (const std::runtime_error &) {
	parse_failure = true;
    }

    REQUIRE(parse_failure);
    
    parser::run("to_num(cc); select(cc >= 0 && (B == 't' || D == 't')); !cols(B, D); sort(cc, aa:r); to_str(cc); cols(aa, bb, cc);", e);
    e.finalize();

    e.set_header(csv::parse_header("A,B,C,D,aa,bb,cc"));

    REQUIRE(e.string() == R"(
stage: 1 (exec_order: 0)
1.1 to_num:
	6 : cc
1.2 select:
	cc 0 >= B t == D t == || && 

1.3 cols:exclude:
	B
	D

stage: 2 (exec_order: 2)
2.1 sort:
	4 : cc
	2 : aa(descending)

stage: 3 (exec_order: 0)
3.1 to_str:
	4 : cc
3.2 cols:keep:
	2 : aa
	3 : bb
	4 : cc
)");
}
