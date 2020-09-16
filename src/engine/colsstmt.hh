#include "../cli_args.hh"
#include "stmt.hh"
#include <algorithm>
#include <stack>
#include <stdexcept>

namespace engine {

class colsstmt : public stmt {
  private:
    std::vector<std::string> columns;
    std::vector<int> col_pos;

    bool exclude{false};

    void set_exclude_header(models::header_row &h);
    const cli_args args;

  public:
    colsstmt(const cli_args &args) : args(args) {}

    void add_ident(const std::string &col) override;
    void add_bang() override;
    std::string string() override;

    void set_header(models::header_row &h) override;
    bool apply(models::row &row, std::stack<models::value> &eval_stack) override;
};

} // namespace engine
