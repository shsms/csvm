#include "to_str_stmt.hh"

namespace engine {

void to_str_stmt::add_ident(const std::string &col) { columns.push_back(col); }

std::string to_str_stmt::string() {
    std::string ret = "to_str:\n";
    if (col_pos.size() == columns.size()) {
        for (auto ii = 0; ii < col_pos.size(); ii++) {
            ret +=
                "\t" + std::to_string(col_pos[ii]) + " : " + columns[ii] + "\n";
        }
    } else {
        for (auto &col : columns) {
            ret += "\t" + col + "\n";
        }
    }
    return ret;
}

models::header_row to_str_stmt::set_header(const models::header_row &h) {
    models::header_row out_headers;
    for (auto &col : columns) {
        bool found = false;
        for (auto ii = 0; ii < h.size(); ii++) {
            if (col == h[ii].name) {
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

bool to_str_stmt::apply(models::row &row) {
    for (auto pos : col_pos) {
	models::to_str(row[pos]);
    }
    return true;
}

} // namespace engine