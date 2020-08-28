#include "csv/csv.hh"
#include "engine/engine.hh"
#include "parser/parser.hh"
#include "order.hh"
#include <fstream>
#include <iostream>
#include <thread>

std::string next_chunk(std::ifstream &file) {
    std::string chunk;

    auto beginning = file.tellg();
    file.seekg(0, std::ios::end);
    auto end = file.tellg();
    auto chunksize = end - beginning;

    if (chunksize > 1e6)
        chunksize = 1e6;
    chunk.resize(chunksize);
    file.seekg(beginning);
    file.read(&chunk[0], chunk.size());
    if (!file.eof()) {
        std::string line;
        std::getline(file, line);
        if (line.length() > 0)
            chunk += std::move(line);
    }
    return chunk;
}

int main(int argc, char *argv[]) {
    if (argc == 2) {
        engine::engine e;
        parser::run(argv[1], e);
        // fmt::print(e.string());

        std::ifstream file("tq.csv", std::ios::in | std::ios::binary);
        if (e.has_header() == false) {
            std::string header;
            std::getline(file, header);
            if (header.length() == 0)
                throw std::runtime_error("couldn't read header");
            csv::parse_header(e, std::move(header));
        }

	ordering_lock lock;

	std::vector<std::thread> threads;
        for (int chunk_id = 0; !file.eof(); chunk_id++) {
	    auto chunk = next_chunk(file);
	    lock.t_start();
	    auto th = std::thread([&](std::string&& data, int id){
		csv::parse_body(e, std::move(data), id, lock);
	    }, std::move(chunk), chunk_id);
	    threads.push_back(std::move(th));
	    // csv::parse_body(e, next_chunk(file), chunk_id, lock);
	}
	for (auto &t : threads) {
	    if (t.joinable())
		t.join();
	}
    }
}
