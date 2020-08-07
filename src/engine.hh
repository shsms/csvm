#ifndef CSVQ_ENGINE_H
#define CSVQ_ENGINE_H

#include <memory>
#include <string>
#include <vector>
#include <algorithm>
#include "colsstmt.hh"
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
    
    void add_bang() { curr_stmt->add_bang();};
    
    std::string string() {
        std::string ret;
        int ctr = 1;
        std::ranges::for_each(curr_block, [&](auto &a) {
            ret += std::to_string(ctr++) + ". " + a->string();
        });
        return ret;
    };
};
} // namespace engine

#endif
