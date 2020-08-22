#include "stmt.hh"
#include <algorithm>
#include <fmt/format.h>
#include <stdexcept>
#include <stack>

namespace engine {

class colsstmt : public stmt {
  private:
    std::vector<std::string> columns;
    std::vector<int> col_pos;

    bool exclude;

    void set_exclude_header(models::header_row &h);

  public:
    colsstmt();
    virtual ~colsstmt(){};

    void add_ident(const std::string &col) override;
    void add_bang() override;
    std::string string() override;

    void set_header(models::header_row &h) override;
    bool apply(models::row &row, std::stack<models::value>& eval_stack) const override;
};

} // namespace engine
