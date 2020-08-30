#include "csv/csv.hh"
#include "engine/engine.hh"
#include "order.hh"
#include "parser/parser.hh"
#include "print.hh"
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

void worker(threading::queue &queue, engine::engine &e,
            threading::ordering_lock &lock, threading::queue &print_queue) {
    auto chunk = queue.dequeue();
    while (chunk.has_value()) {
        csv::parse_body(e, std::move(chunk->data), chunk->id, lock,
                        print_queue);
        chunk = queue.dequeue();
    }
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

    engine::engine e;
    parser::run(script, e);
    // fmt::print(e.string());
    std::ifstream file(filename, std::ios::in | std::ios::binary);
    if (!e.has_header()) {
        std::string header;
        std::getline(file, header);
        if (header.length() == 0) {
            throw std::runtime_error("couldn't read header");
        }
        // TODO: make parse_header return hrow, assign here,
        // instead of passing the engine to it.  then make
        // csv::parse_body take const reference to engine.
        csv::parse_header(e, std::move(header));
    }

    if (thread_count < 1) {
        thread_count = 1;
    }

    threading::ordering_lock lock;
    threading::queue input_queue(queue_size);
    threading::queue print_queue(queue_size);

    std::thread print_thread([&]() { threading::print_worker(print_queue); });

    std::vector<std::thread> threads;
    threads.reserve(thread_count);
    for (int ii = 0; ii < thread_count; ii++) {
        threads.push_back(
            std::thread([&]() { worker(input_queue, e, lock, print_queue); }));
    }

    for (int chunk_id = 0; !file.eof(); chunk_id++) {
        input_queue.enqueue({chunk_id, next_chunk(file, chunk_size)});
    }
    input_queue.set_eof();

    for (auto &t : threads) {
        if (t.joinable()) {
            t.join();
        }
    }

    print_queue.set_eof();

    if (print_thread.joinable()) {
        print_thread.join();
    }
}
