#include "to_num_stmt.hh"

namespace engine {

void to_num_stmt::add_ident(const std::string &col) { columns.push_back(col); }

std::string to_num_stmt::string() {
    std::string ret = "to_num:\n";
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

void to_num_stmt::set_header(models::header_row &h) {
    for (auto &col : columns) {
        bool found = false;
        for (auto ii = 0; ii < h.size(); ii++) {
            if (col == h[ii].name) {
                col_pos.push_back(ii);
                found = true;
                break;
            }
        }
        if (found == false) {
            throw std::runtime_error("column not found in header:" + col);
        }
    }
}

bool to_num_stmt::apply(models::row &row) {
    for (auto pos : col_pos) {
	models::to_num(row[pos]);
    }
    return true;
}

} // namespace engine
