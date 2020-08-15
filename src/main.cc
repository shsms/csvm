#include "csv/csv.hh"
#include "engine/engine.hh"
#include "parser/parser.hh"
#include <iostream>

int main(int argc, char *argv[]) {
    if (argc == 2) {
        engine::engine e;
        parser::run(argv[1], e);
	//fmt::print(e.string());
        csv::run("tq.csv", e);
    }
}
