#include "cli_args.hh"
#include "csv/csv.hh"
#include "engine/engine.hh"
#include "input.hh"
#include "parser/parser.hh"
#include "threading/queue.hh"
#include <fstream>
#include <iostream>
#include <thread>

int main(int argc, char *argv[]) {
    auto args = parse_args(argc, argv);

    engine::engine e(args);
    parser::run(args.script, e);
    auto file = std::fstream(args.in_filename, std::ios::in | std::ios::binary);
    e.finalize();

    if (!e.has_header()) {
        std::string header_raw;
        std::getline(file, header_raw);
        if (header_raw.length() == 0) {
            std::cerr << "couldn't read header from file\n";
            return -42;
        }
        e.set_header(csv::parse_header(std::move(header_raw)));
    }

    if (args.print_engine) {
        std::cerr << e.string();
        return 0;
    }

    e.start();

    pthread_setname_np(pthread_self(), "csvm_main");

    auto &input_queue = e.get_input_queue();

    for (int chunk_id = 0; !file.eof(); ++chunk_id) {
        input_queue.enqueue({chunk_id, next_chunk(file, args.chunk_size)});
    }

    e.cleanup();
}
