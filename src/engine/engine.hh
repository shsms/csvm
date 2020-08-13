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

  public:
    void new_cols_stmt();
    void finish_stmt();
    void add_ident(const std::string &ident);
    void add_bang();
    void apply(models::row &row);
    std::string string();
    void set_header(const models::row &h);
};
} // namespace engine

#endif