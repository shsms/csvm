#include "csv/csv.hh"
#include "engine/engine.hh"
#include "parser/parser.hh"
#include <iostream>

int main(int argc, char *argv[]) {
    if (argc == 2) {
        engine::engine e;
	fmt::print(e.string());
        parser::run(argv[1], e);
        csv::run("test.csv", e);
    }
}
