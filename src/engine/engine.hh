#ifndef CSVM_ENGINE_H
#define CSVM_ENGINE_H

#include "../cli_args.hh"
#include "../threading/queue.hh"
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

struct tblock {
    stmt::exec_order exec_order{};
    std::vector<std::shared_ptr<stmt>> stmts;
};

using thread_group = std::vector<std::thread>;
class engine {
    std::shared_ptr<stmt> curr_stmt; // TODO: is unique_ptr possible here?
    tblock curr_block;
    std::vector<tblock> tblocks;
    bool header_set = false;

    const cli_args args;

    threading::raw_queue input_queue;
    threading::raw_queue print_queue;
    std::vector<threading::bin_queue> block_queues;
    std::vector<thread_group> thread_groups;
    std::thread print_thread;
    stmt::exec_order prev_exec_order{stmt::curr_block};

  public:
    engine(const cli_args &args) : args(args) {}

    template <typename T> void new_stmt() {
        curr_stmt = std::static_pointer_cast<stmt>(std::make_shared<T>(args));
    }
    void finish_stmt();

    void add_ident(const std::string &);
    void add_str(const std::string &);
    void add_num(const std::string &);
    void add_bang();
    void add_oper(const std::string &);
    void finalize();

    void start();
    void cleanup();
    inline auto &get_input_queue() { return input_queue; }

    std::string string();
    bool has_header() const;
    void set_header(models::header_row &&h);
};

bool apply(const tblock &, models::row &, std::stack<models::value> &);
bool apply(const tblock &, models::bin_chunk &, std::stack<models::value> &);

} // namespace engine

#endif
