#include "engine.hh"
#include "../csv/csv.hh"
#include "../models/models.hh"
#include "colsstmt.hh"
#include "selectstmt.hh"
#include <iostream>

namespace engine {

models::raw_chunk to_raw_chunk(models::bin_chunk &bin_chunk) {
    static const std::string comma_str = ",";
    static const std::string newline = "\n";
    std::string print_buffer;
    for (int ii = 0; ii < bin_chunk.length; ii++) {
	if (!bin_chunk.data[ii].second) {
	    continue;
	}
	const auto &row = bin_chunk.data[ii];
        for (auto jj = 0; jj < row.first.size(); jj++) {
            if (jj == 0) {
                print_buffer += std::get<std::string>(row.first[jj]);
            } else {
                print_buffer += comma_str + std::get<std::string>(row.first[jj]);
            }
        }
        print_buffer += newline;
    }

    return {bin_chunk.id, std::move(print_buffer)};
}

void worker(threading::queue<models::raw_chunk> &queue, const engine &e,
            threading::queue<models::raw_chunk> &print_queue) {
    std::stack<models::value> tmp_eval_stack;
    models::bin_chunk parsed;
    
    auto chunk = queue.dequeue();
    while (chunk.has_value()) {
        csv::parse_body(std::move(chunk.value()), parsed);
        for (int ii = 0;  ii < parsed.length; ii++) {
            parsed.data[ii].second = e.apply(parsed.data[ii], tmp_eval_stack);
        }
        print_queue.enqueue(to_raw_chunk(parsed));
        chunk = queue.dequeue();
    }
}

void print_worker(threading::queue<models::raw_chunk> &queue) {
    int next = 0;
    std::unordered_map<int, std::string> items;

    auto chunk = queue.dequeue();
    while (chunk.has_value()) {
        if (chunk->id == next) {
            std::cout << chunk->data;
            ++next;
            while (items.count(next) > 0) {
                std::cout << items.at(next);
                // TODO:  reusing strings helps avoid reallocation
                // probably has an impact only in super large files.
                items.erase(next);
                ++next;
            }
        } else {
            items.emplace(chunk->id, std::move(chunk->data));
        }
        chunk = queue.dequeue();
    }
}

void engine::finish_stmt() {
    curr_stmt->finalize();
    curr_block.emplace_back(curr_stmt);
};

void engine::add_ident(const std::string &ident) {
    curr_stmt->add_ident(ident);
};

void engine::add_str(const std::string &str) { curr_stmt->add_str(str); };

void engine::add_num(const std::string &str) { curr_stmt->add_num(str); };

void engine::add_bang() { curr_stmt->add_bang(); };

void engine::add_oper(const std::string &oper) { curr_stmt->add_oper(oper); }

bool engine::apply(models::row &row,
                   std::stack<models::value> &eval_stack) const {
    bool keep = true;
    for (const auto &s : curr_block) {
        keep = s->apply(row, eval_stack);
        if (!keep) {
            return false;
        }
    }
    return true;
}

std::string engine::string() {
    std::string ret;
    int ctr = 1;
    for (auto &s : curr_block) {
        ret += std::to_string(ctr++) + ". " + s->string();
    }
    return ret;
};

void engine::set_header(models::header_row &&h) {
    header_set = true;
    for (auto &s : curr_block) {
        s->set_header(h);
    }
    std::string buffer{};
    for (auto ii = 0; ii < h.size(); ii++) {
        if (ii == 0) { // TODO compare with print first col outside loop
            buffer += h[ii].name;
        } else {
            buffer += "," + h[ii].name;
        }
    }
    buffer += "\n";
    std::cout << buffer;
}

bool engine::has_header() const { return header_set; }

void engine::start() {
    input_queue.set_limit(in_queue_size);
    print_queue.set_limit(out_queue_size);

    print_thread = std::thread([&]() { print_worker(print_queue); });

    if (thread_count < 1) {
        thread_count = 1;
    }

    worker_threads.reserve(thread_count);

    for (int ii = 0; ii < thread_count; ii++) {
        worker_threads.push_back(
            std::thread([&]() { worker(input_queue, *this, print_queue); }));
    }
}

void engine::cleanup() {
    input_queue.set_eof();

    for (auto &t : worker_threads) {
        if (t.joinable()) {
            t.join();
        }
    }

    print_queue.set_eof();

    if (print_thread.joinable()) {
        print_thread.join();
    }
}

} // namespace engine
