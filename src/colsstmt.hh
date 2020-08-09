#include "stmt.hh"
#include <algorithm>
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
    colsstmt() { exclude = false; }

    virtual ~colsstmt(){};

    void add_ident(const std::string &col) { columns.push_back(col); }

    std::string string() {
        std::string excl = "keep:";
        if (exclude == true)
            excl = "exclude:";
        std::string ret = "cols:" + excl + "\n";
        if (col_pos.size() == columns.size()) {
            for (auto ii = 0; ii < col_pos.size(); ii++) {
                ret += "\t" + std::to_string(col_pos[ii]) + " : " +
                       columns[ii] + "\n";
            }
        } else {
            for (auto &col : columns) {
                ret += "\t" + col + "\n";
            }
        }
        return ret;
    }

    void add_bang() { exclude = true; }

    models::row set_header(const models::row &h) {
        for (auto &col : columns) {
            bool found = false;
            for (auto ii = 0; ii < h.size(); ii++) {
                if (col == h[ii]) {
                    col_pos.push_back(ii);
                    out_headers.push_back(h[ii]);
                    found = true;
                    break;
                }
            }
            if (found == false) {
                throw std::runtime_error("column not found in header:" + col);
            }
        }
        return out_headers;
    }

    models::row apply(const models::row &row) {
        models::row ret;
        for (auto pos : col_pos) {
            ret.push_back(row[pos]);
        }
        return ret;
    }
};
} // namespace engine
