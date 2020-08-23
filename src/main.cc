#include "csv/csv.hh"
#include "engine/engine.hh"
#include "parser/parser.hh"
#include <iostream>
#include <fstream>

int main(int argc, char *argv[]) {
    if (argc == 2) {
        engine::engine e;
        parser::run(argv[1], e);
	//fmt::print(e.string());

	std::string contents;
	std::ifstream file("tq.csv", std::ios::in | std::ios::binary);
	if (e.has_header() == false) {
	    std::string header;
	    std::getline(file, header);
	    if (header.length() == 0)
		throw std::runtime_error("couldn't read header");
	    csv::parse_header(e, std::move(header));
	}
	auto beginning = file.tellg();
	file.seekg(0, std::ios::end);
	contents.resize(file.tellg() - beginning);
	file.seekg(beginning);
	file.read(&contents[0], contents.size());

	csv::parse_body(e, std::move(contents), -1);
    }
}
