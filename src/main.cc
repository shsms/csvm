#include "parser/parser.hh"
#include "engine.hh"
#include <iostream>

int main(int argc, char *argv[]) {
    if  (argc == 2) {
	engine::engine e;
	parser::run(argv[1], e);
	std::cout << e.string();
    }
}
