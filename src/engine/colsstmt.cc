#include "colsstmt.hh"

namespace engine {

colsstmt::colsstmt() { exclude = false; }

void colsstmt::add_ident(const std::string &col) { columns.push_back(col); }

void colsstmt::add_bang() { exclude = true; }

std::string colsstmt::string() {
    std::string excl = "keep:";
    if (exclude == true)
        excl = "exclude:";
    std::string ret = "cols:" + excl + "\n";
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

models::row colsstmt::set_header(const models::row &h) {
    if (exclude == true) {
        return set_exclude_header(h);
    }
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

models::row colsstmt::set_exclude_header(const models::row &h) {
    for (auto ii = 0; ii < h.size(); ii++) {
        bool found = false;
        for (auto &col : columns) {
            if (col == h[ii]) {
                found = true;
                break;
            }
        }
        if (found == false) {
            col_pos.push_back(ii);
            out_headers.push_back(h[ii]);
        }
    }
    return out_headers;
}

models::row colsstmt::apply(const models::row &row) {
    models::row ret;
    for (auto pos : col_pos) {
        ret.push_back(row[pos]);
    }
    return ret;
}

} // namespace engine
