#include "stmt.hh"
#include <algorithm>
#include <stdexcept>

namespace engine {

class colsstmt : public stmt {
  private:
    std::vector<std::string> columns;
    std::vector<int> col_pos;

    bool exclude;

    models::header_row set_exclude_header(const models::header_row &h);

  public:
    colsstmt();
    virtual ~colsstmt(){};

    void add_ident(const std::string &col) override;
    void add_bang() override;
    std::string string() override;

    models::header_row set_header(const models::header_row &h) override;
    bool apply(models::row &row) override;
};

} // namespace engine
