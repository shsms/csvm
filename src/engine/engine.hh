#ifndef CSVQ_ENGINE_H
#define CSVQ_ENGINE_H

#include "stmt.hh"
#include <algorithm>
#include <memory>
#include <string>
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
    std::string print_buffer;

  public:
    template <class T> void new_stmt() {
        curr_stmt = std::static_pointer_cast<stmt>(std::make_shared<T>());
    }
    void finish_stmt();

    void add_ident(const std::string &);
    void add_str(const std::string &);
    void add_num(const std::string &);
    void add_bang();
    void add_oper(const std::string &);
    void begin_method(const std::string &);
    void end_method();

    void apply(models::row &row);
    void cleanup();
    std::string string();
    void set_header(const models::header_row &h);
};
} // namespace engine

#endif
