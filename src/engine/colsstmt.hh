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

    models::row set_exclude_header(const models::row &h);
  public:
    colsstmt();
    virtual ~colsstmt(){};

    void add_ident(const std::string &col);
    void add_str(const std::string&) {}
    void add_oper(const std::string &) {}
    void add_bang();
    void finalize() {};
    std::string string();
    
    models::row set_header(const models::row &h);
    models::row apply(const models::row &row);
};

} // namespace engine
