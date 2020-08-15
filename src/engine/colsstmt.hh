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

    void add_ident(const std::string &col) override;
    void add_str(const std::string &) override {}
    void add_num(const std::string &) override {}
    void add_oper(const std::string &) override {}
    void add_bang() override;
    void finalize() override{};
    std::string string() override;

    models::row set_header(const models::row &h) override;
    bool apply(models::row &row) override;
};

} // namespace engine
