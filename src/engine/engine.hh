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
    bool header_set = false;
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

    bool apply(models::row &row);
    std::string string();
    bool has_header();
    void set_header(models::header_row &h);
};
} // namespace engine

#endif
