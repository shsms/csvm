#ifndef CSVM_SORTSTMT_HH
#define CSVM_SORTSTMT_HH

#include "stmt.hh"
#include <algorithm>
#include <fmt/format.h>
#include <stack>
#include <stdexcept>

namespace engine {

class sortstmt : public stmt {
  private:
    int curr_pos{};
    struct colspec {
        bool descending;
        bool numeric;
        int pos;
        std::string name;
    };
    std::vector<colspec> columns;

  public:
    void add_ident(const std::string &col) override;
    void
    add_oper(const std::string &oper) override; // only for reverse "r" oper.
    std::string string() override;

    inline stmt::exec_order finalize() override { return sep_block; }

    void set_header(models::header_row &h) override;
    bool apply(models::bin_chunk &chunk,
               std::stack<models::value> &eval_stack) override;
};

} // namespace engine

#endif /* CSVM_SORTSTMT_HH */
