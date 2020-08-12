#include "stmt.hh"
#include <algorithm>
#include <fmt/format.h>
#include <stdexcept>

namespace engine {

class colsstmt : public stmt {
  private:
    std::vector<std::string> columns;
    std::vector<int> col_pos;
    models::row in_headers;
    models::row out_headers;

    bool exclude;

  public:
    colsstmt();
    virtual ~colsstmt(){};

    void add_ident(const std::string &col);
    std::string string();
    void add_bang();
    models::row set_header(const models::row &h);
    models::row set_exclude_header(const models::row &h);
    models::row apply(const models::row &row);
};

} // namespace engine
