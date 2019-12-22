#include <memory>
#include <string>
#include <vector>

namespace sm {
class stmt {
  public:
    virtual void add_ident(const std::string &ident) = 0;
    virtual void add_type(const std::string &type) = 0;
};

class colsstmt : stmt {};

// TODO: tblock should eventually be a class that has queues to
// connect to other subsequent tblocks.  Testing only single threaded mode right now.
typedef std::vector<std::shared_ptr<stmt>> tblock;

class statemachine {
    std::shared_ptr<stmt> curr_stmt;
    tblock curr_block;
    std::vector<tblock> tblocks;
  public:
    void new_cols_stmt();
    void add_ident(const std::string &ident);
    void add_type(const std::string &type);
};
} // namespace sm
