#include "parser/parser.hh"

int main(int argc, char *argv[]) {
    if  (argc == 2) {
	parser::run(argv[1]);
    }
}
