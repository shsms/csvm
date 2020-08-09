#ifndef CSVQ_ENGINE_H
#define CSVQ_ENGINE_H

#include "colsstmt.hh"
#include "models.hh"
#include <algorithm>
#include <fmt/format.h>
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
    void new_cols_stmt() {
        curr_stmt =
            std::static_pointer_cast<stmt>(std::make_shared<colsstmt>());
    };

    void finish_stmt() { curr_block.emplace_back(curr_stmt); };

    void add_ident(const std::string &ident) { curr_stmt->add_ident(ident); };

    void add_bang() { curr_stmt->add_bang(); };

    void apply(models::row &row) {
        auto nextrow = row;
        for (auto &s : curr_block) {
            nextrow = s->apply(nextrow);
        }
        for (auto &val : nextrow)
            fmt::print("{},", val);
        fmt::print("\n");
    }

    std::string string() {
        std::string ret;
        int ctr = 1;
        for (auto &s : curr_block) {
            ret += std::to_string(ctr++) + ". " + s->string();
        }
        return ret;
    };

    void set_header(const models::row &h) {
        auto nextrow = h;
        for (auto &s : curr_block) {
            nextrow = s->set_header(nextrow);
        }
        for (auto &val : nextrow)
            fmt::print("{},", val);
        fmt::print("\n");
    }
};
} // namespace engine

#endif
