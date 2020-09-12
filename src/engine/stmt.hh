#ifndef CSVM_STMT_H
#define CSVM_STMT_H

#include "../models/models.hh"
#include "../threading/queue.hh"
#include <stack>
#include <string>

namespace engine {

class stmt {
  public:
    virtual ~stmt(){};
    virtual void set_header(models::header_row &) = 0;
    virtual void set_thread_count(int c) {}

    virtual void add_ident(const std::string &) = 0;

    virtual void add_str(const std::string & /*unused*/) {
        throw std::runtime_error("stmt::add_str not implemented");
    }

    virtual void add_num(const std::string & /*unused*/) {
        throw std::runtime_error("stmt::add_num not implemented");
    }

    virtual void add_bang() {
        throw std::runtime_error("stmt::add_bang not implemented");
    }

    virtual void add_oper(const std::string & /*unused*/) {
        throw std::runtime_error("stmt::add_oper not implemented");
    }

    enum exec_order {
        curr_block,
        new_block,
        sep_block // run in a separate block - sort, stats, etc.
    };

    virtual exec_order finalize() { return curr_block; }

    virtual std::string string() = 0;

    virtual bool apply(models::row &row,
                       std::stack<models::value> &eval_stack) {
        throw std::runtime_error("stmt::apply<row> not implemented");
    }

    virtual bool apply(models::bin_chunk &chunk,
                       std::stack<models::value> &eval_stack) {
        throw std::runtime_error("stmt::apply<chunk> not implemented");
    }

    virtual bool
    run_worker(threading::bin_queue &in_queue,
               const std::function<void(models::bin_chunk &)> &forwarder) {
        return false;
    }
};

} // namespace engine
#endif
