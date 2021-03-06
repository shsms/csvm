#include "engine.hh"
#include "../csv/csv.hh"
#include "../models/models.hh"
#include <fstream>
#include <iostream>

namespace engine {

void entry_exit_worker(threading::raw_queue &queue, const tblock &block,
                       threading::raw_queue &print_queue) {
    pthread_setname_np(pthread_self(), "csvm_entry_exit");
    std::stack<models::value> tmp_eval_stack;
    std::string print_buffer;
    auto chunk = queue.dequeue();
    while (chunk.has_value()) {
        csv::parse_body(std::move(chunk.value()), [&](models::row &row) {
            if (apply(block, row, tmp_eval_stack)) {
                models::append_to_string(print_buffer, row);
            }
        });
        print_queue.enqueue({chunk->id, std::move(print_buffer)});
        print_buffer.clear();
        chunk = queue.dequeue();
    }
}

void entry_worker(threading::raw_queue &in_queue, const tblock &block,
                  threading::bin_queue &out_queue) {
    pthread_setname_np(pthread_self(), "csvm_entry");
    std::stack<models::value> tmp_eval_stack;
    models::bin_chunk out_chunk;
    auto in_chunk = in_queue.dequeue();
    while (in_chunk.has_value()) {
        out_chunk.id = in_chunk->id;
        csv::parse_body(std::move(in_chunk.value()), [&](models::row &row) {
            if (apply(block, row, tmp_eval_stack)) {
                out_chunk.data.emplace_back(std::move(row));
            }
        });
        out_queue.enqueue(std::move(out_chunk));
        out_chunk.data.clear();
        in_chunk = in_queue.dequeue();
    }
}

void intermediate_worker(threading::bin_queue &in_queue, const tblock &block,
                         threading::bin_queue &out_queue) {
    pthread_setname_np(pthread_self(), "csvm_middle");
    // TODO: make forwarder move-only
    auto forwarder = [&out_queue](models::bin_chunk &c) { out_queue.enqueue(std::move(c)); };
    if (block.exec_order == stmt::sep_block && block.stmts[0]->run_worker(in_queue, forwarder)) {
        return;
    }
    std::stack<models::value> tmp_eval_stack;
    models::bin_chunk out_chunk;
    auto in_chunk = in_queue.dequeue();
    while (in_chunk.has_value()) {
        out_chunk.id = in_chunk->id;
        for (auto &row : in_chunk.value().data) {
            if (apply(block, row, tmp_eval_stack)) {
                out_chunk.data.emplace_back(row);
            }
        }
        out_queue.enqueue(std::move(out_chunk));
        out_chunk.data.clear();
        in_chunk = in_queue.dequeue();
    }
}

void exit_worker(threading::bin_queue &in_queue, const tblock &block,
                 threading::raw_queue &print_queue) {
    pthread_setname_np(pthread_self(), "csvm_exit");
    std::string print_buffer;

    auto forwarder = [&print_queue, &print_buffer](models::bin_chunk &c) {
        for (const auto &row : c.data) {
            models::append_to_string(print_buffer, row);
        }
        print_queue.enqueue({c.id, std::move(print_buffer)});
        print_buffer.clear();
    };

    if (block.exec_order == stmt::sep_block && block.stmts[0]->run_worker(in_queue, forwarder)) {
        return;
    }

    std::stack<models::value> tmp_eval_stack;
    std::string out_buffer;
    auto in_chunk = in_queue.dequeue();
    while (in_chunk.has_value()) {
        for (auto &row : in_chunk.value().data) {
            if (apply(block, row, tmp_eval_stack)) {
                models::append_to_string(out_buffer, row);
            }
        }
        print_queue.enqueue({in_chunk->id, std::move(out_buffer)});
        in_chunk = in_queue.dequeue();
    }
}

void output_worker(threading::queue<models::raw_chunk> &queue, const std::string &out_filename) {
    pthread_setname_np(pthread_self(), "csvm_output");

    std::ofstream fs;
    fs.open(out_filename, std::ios::trunc | std::ios::out | std::ios::binary);

    int next = -1; // chunk number -1 is the header.
    std::unordered_map<int, std::string> items;
    auto chunk = queue.dequeue();
    while (chunk.has_value()) {
        if (chunk->id == next) {
            fs.write(chunk->data.c_str(), chunk->data.length());
            if (fs.bad()) {
                throw std::runtime_error(std::string("unable to write to output file '") +
                                         out_filename + "'");
            }

            ++next;
            while (items.count(next) > 0) {
                fs.write(items.at(next).c_str(), items.at(next).length());
                if (fs.bad()) {
                    throw std::runtime_error(std::string("unable to write to output file '") +
                                             out_filename + "'");
                }
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
    auto exec_order = curr_stmt->finalize();
    if (exec_order == stmt::sep_block || prev_exec_order == stmt::sep_block) {
        tblocks.emplace_back(std::move(curr_block));
        curr_block.stmts.clear();
        curr_block.exec_order = exec_order;
        curr_block.stmts.emplace_back(curr_stmt);
        block_queues.emplace_back(threading::bin_queue{});
        block_queues.back().set_limit(args.thread_count);
    } else if (exec_order == stmt::curr_block) {
        curr_block.exec_order = exec_order;
        curr_block.stmts.emplace_back(curr_stmt);
    }
    prev_exec_order = exec_order;
};

void engine::add_ident(const std::string &ident) { curr_stmt->add_ident(ident); };

void engine::add_str(const std::string &str) { curr_stmt->add_str(str); };

void engine::add_num(const std::string &str) { curr_stmt->add_num(str); };

void engine::add_bang() { curr_stmt->add_bang(); };

void engine::add_oper(const std::string &oper) { curr_stmt->add_oper(oper); }

bool apply(const tblock &block, models::row &row, std::stack<models::value> &eval_stack) {
    bool keep = true;
    for (const auto &s : block.stmts) {
        keep = s->apply(row, eval_stack);
        if (!keep) {
            return false;
        }
    }
    return true;
}

bool apply(const tblock &block, models::bin_chunk &chunk, std::stack<models::value> &eval_stack) {
    bool keep = true;
    for (const auto &s : block.stmts) {
        keep = s->apply(chunk, eval_stack);
        if (!keep) {
            return false;
        }
    }
    return true;
}

std::string engine::string() {
    std::string ret;
    int bctr = 0;
    int sctr = 0;
    for (auto &block : tblocks) {
        ++bctr;
        sctr = 0;
        ret += "\nstage: " + std::to_string(bctr) +
               " (exec_order: " + std::to_string(block.exec_order) + ")\n";
        for (auto &s : block.stmts) {
            ret += std::to_string(bctr) + "." + std::to_string(++sctr) + " " + s->string();
        }
    }
    return ret;
};

void engine::set_header(models::header_row &&h) noexcept {
    header_set = true;
    for (auto &block : tblocks) {
        for (auto &s : block.stmts) {
            s->set_header(h);
        }
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
    output_queue.enqueue(models::raw_chunk{-1, buffer});
}

bool engine::has_header() const { return header_set; }

void engine::finalize() {
    tblocks.emplace_back(curr_block);

    for (auto &block : tblocks) {
        for (auto &s : block.stmts) {
            s->set_thread_count(args.thread_count);
        }
    }
}

void engine::start() {
    // TODO: use narrow info from thread-distributor once it is ready.
    input_queue.set_limit(args.thread_count);
    output_queue.set_limit(args.thread_count);

    print_thread = std::thread([&]() { output_worker(output_queue, args.out_filename); });

    if (tblocks.size() == 1) {
        thread_groups.resize(1);
        for (int ii = 0; ii < args.thread_count; ++ii) {
            thread_groups.front().emplace_back(std::thread(
                [&]() { entry_exit_worker(input_queue, tblocks.front(), output_queue); }));
        }
    } else {
        thread_groups.resize(tblocks.size());
        for (int ii = 0; ii < args.thread_count; ++ii) {
            thread_groups.front().emplace_back(std::thread(
                [&]() { entry_worker(input_queue, tblocks.front(), block_queues[0]); }));
            for (auto jj = 0; jj < tblocks.size() - 2; ++jj) {
                thread_groups[jj + 1].emplace_back(std::thread([&, jj]() {
                    intermediate_worker(block_queues[jj], tblocks[jj + 1], block_queues.at(jj + 1));
                }));
            }
            thread_groups.back().emplace_back(std::thread(
                [&]() { exit_worker(block_queues.back(), tblocks.back(), output_queue); }));
        }
    }
}

void engine::cleanup() {
    input_queue.set_eof();

    for (int ii = 0; ii < thread_groups.size(); ii++) {
        if (ii > 0) {
            block_queues[ii - 1].set_eof();
        }
        for (auto &t : thread_groups[ii]) {
            if (t.joinable()) {
                t.join();
            }
        }
    }

    output_queue.set_eof();

    if (print_thread.joinable()) {
        print_thread.join();
    }
}

} // namespace engine
