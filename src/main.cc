#include "csv/csv.hh"
#include "engine/engine.hh"
#include "parser/parser.hh"
#include "threading/queue.hh"
#include <CLI/CLI.hpp>
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
    CLI::App app;

    std::string filename{"/dev/stdin"};
    std::string script;
    int thread_count{1};
    int in_queue_size{};
    int out_queue_size{};
    double chunk_size{1e6};
    bool print_engine{false};
    app.add_option(
           "-f", filename,
           "csv filename, required until next_stdin_chunk is implemented")
        ->required()
        ->check(CLI::ExistingFile);
    app.add_option("script", script, "script to execute")->required();
    app.add_option("-n", thread_count, "number of threads, defaults to 1");
    app.add_option("--in-queue-size", in_queue_size,
                   "defaults to number of threads");
    app.add_option("--out-queue-size", out_queue_size,
                   "defaults to number of threads");
    app.add_option("--chunk_size", chunk_size,
                   "size in bytes of each input chunk, defaults to 1e6");
    app.add_flag("--print-engine", print_engine,
                 "display how the engine is built and exit");
    try {
        app.parse(argc, argv);
    } catch (const CLI::ParseError &e) {
        // questions creates imbalance, and answers that fit resolve them.
        // if 42 is the answer,  then the question has to be -42.
        // return -42;
        return app.exit(e);
    }

    if (chunk_size <= 0) {
        chunk_size = 1e6;
    }
    if (thread_count <= 0) {
        thread_count = 1;
    }
    if (in_queue_size <= 0) {
        in_queue_size = thread_count;
    }
    if (out_queue_size <= 0) {
        out_queue_size = thread_count;
    }

    engine::engine e(thread_count, in_queue_size, out_queue_size);
    parser::run(script, e);
    auto file = std::ifstream(filename, std::ios::in | std::ios::binary);

    e.finalize();
    // std::cerr << e.string();
    if (!e.has_header()) {
        std::string header_raw;
        std::getline(file, header_raw);
        if (header_raw.length() == 0) {
            std::cerr << "couldn't read header from file\n";
            return -42;
        }
        e.set_header(csv::parse_header(std::move(header_raw)));
    }

    if (print_engine) {
        std::cerr << e.string();
        return 0;
    }

    e.start();

    auto &input_queue = e.get_input_queue();

    for (int chunk_id = 0; !file.eof(); ++chunk_id) {
        input_queue.enqueue({chunk_id, next_chunk(file, chunk_size)});
    }

    e.cleanup();
}
