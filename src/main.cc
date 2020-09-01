#include "csv/csv.hh"
#include "engine/engine.hh"
#include "order.hh"
#include "parser/parser.hh"
#include "queue.hh"
#include <fstream>
#include <iostream>
#include <thread>

std::string next_chunk(std::ifstream &file, uint64_t max_chunk_size) {
    std::string chunk;

    auto beginning = file.tellg();
    file.seekg(0, std::ios::end);
    auto end = file.tellg();
    auto chunksize = end - beginning;

    if (chunksize > max_chunk_size) {
        chunksize = max_chunk_size;
    }
    chunk.resize(chunksize);
    file.seekg(beginning);
    file.read(&chunk[0], chunk.size());
    if (!file.eof()) {
        std::string line;
        std::getline(file, line);
        if (line.length() > 0) {
            chunk += line;
        }
    }
    return chunk;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        // questions creates imbalance, and answers that fit resolve them.
        // if 42 is the answer,  then the question has to be -42.
        return -42;
    }

    auto *script = argv[1];          // TODO: cliparam
    const auto *filename = "tq.csv"; // TODO: cliparam
    auto thread_count = 4;           // TODO: cliparam
    auto queue_size = 4;             // TODO: cliparam
    auto chunk_size = 5e5;           // TODO: cliparam

    engine::engine e(thread_count, queue_size);
    parser::run(script, e);

    std::ifstream file(filename, std::ios::in | std::ios::binary);
    if (!e.has_header()) {
        std::string header_raw;
        std::getline(file, header_raw);
        if (header_raw.length() == 0) {
            std::cerr << "couldn't read header from file\n";
            return -42;
        }
        e.set_header(csv::parse_header(std::move(header_raw)));
    }

    e.start();

    auto &input_queue = e.get_input_queue();

    for (int chunk_id = 0; !file.eof(); ++chunk_id) {
        input_queue.enqueue({chunk_id, next_chunk(file, chunk_size)});
    }

    e.cleanup();
}
