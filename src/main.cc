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

void worker(threading::queue &queue, engine::engine &e,
            threading::ordering_lock &lock) {
    auto chunk = queue.dequeue();
    while (chunk.has_value()) {
        csv::parse_body(e, std::move(chunk->data), chunk->id, lock);
        chunk = queue.dequeue();
    }
}

int main(int argc, char *argv[]) {
    if (argc == 2) {
        engine::engine e;
        parser::run(argv[1], e);
        // fmt::print(e.string());

        std::ifstream file("tq.csv", std::ios::in | std::ios::binary);
        if (!e.has_header()) {
            std::string header;
            std::getline(file, header);
            if (header.length() == 0) {
                throw std::runtime_error("couldn't read header");
            }
            csv::parse_header(e, std::move(header));
        }

        auto thread_count = 9;
        threading::ordering_lock lock;

        if (thread_count > 1) {
            threading::queue queue(1000);

            std::vector<std::thread> threads;
            threads.reserve(thread_count);
            for (int ii = 0; ii < thread_count; ii++) {
                threads.push_back(
                    std::thread([&]() { worker(queue, e, lock); }));
            }

            for (int chunk_id = 0; !file.eof(); chunk_id++) {
                queue.enqueue({chunk_id, next_chunk(file, 5e6)});
            }
            queue.set_eof();

            for (auto &t : threads) {
                if (t.joinable()) {
                    t.join();
                }
            }
        } else {
            for (int chunk_id = 0; !file.eof(); chunk_id++) {
                csv::parse_body(e, next_chunk(file, 1e6), -1, lock);
            }
        }
    }
}
