#ifndef CSVQ_ENGINE_H
#define CSVQ_ENGINE_H

#include "../queue.hh"
#include "stmt.hh"
#include <algorithm>
#include <memory>
#include <stack>
#include <string>
#include <thread>
#include <vector>
namespace engine {

// TODO: tblock should eventually be a class that has queues to
// connect to other subsequent tblocks.  Testing only single threaded mode right
// now.
using tblock = std::vector<std::shared_ptr<stmt>>;

class engine {
    std::shared_ptr<stmt> curr_stmt;
    tblock curr_block;
    std::vector<tblock> tblocks;
    bool header_set = false;

    int thread_count{}, in_queue_size{}, out_queue_size;

    threading::queue<models::raw_chunk> input_queue;
    threading::queue<models::raw_chunk> print_queue;
    std::vector<std::thread> worker_threads;
    std::thread print_thread;

  public:
    engine(int trd_cnt, int in_q_sz, int out_q_sz)
        : thread_count(trd_cnt), in_queue_size(in_q_sz),
          out_queue_size(out_q_sz) {}

    template <typename T> void new_stmt() {
        curr_stmt = std::static_pointer_cast<stmt>(std::make_shared<T>());
    }
    void finish_stmt();

    void add_ident(const std::string &);
    void add_str(const std::string &);
    void add_num(const std::string &);
    void add_bang();
    void add_oper(const std::string &);

    void start();
    void cleanup();
    inline auto &get_input_queue() { return input_queue; }

    bool apply(models::row &row, std::stack<models::value> &eval_stack) const;
    std::string string();
    bool has_header() const;
    void set_header(models::header_row &&h);
};
} // namespace engine

#endif
