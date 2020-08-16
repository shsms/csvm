#include "stmt.hh"
#include <algorithm>
#include <fmt/format.h>
#include <stdexcept>

namespace engine {

class to_str_stmt : public stmt {
  private:
    std::vector<std::string> columns;
    std::vector<int> col_pos;

  public:
    void add_ident(const std::string &col) override;
    std::string string() override;

    models::header_row set_header(const models::header_row &h) override;
    bool apply(models::row &row) override;
};

} // namespace engine