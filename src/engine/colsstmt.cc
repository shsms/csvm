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

void colsstmt::set_header(models::header_row &h) {
    if (exclude == true) {
        set_exclude_header(h);
	return;
    }
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
    h = std::move(out_headers);
}

void colsstmt::set_exclude_header(models::header_row &h) {
    models::header_row out_headers;
    for (auto ii = 0; ii < h.size(); ii++) {
        bool found = false;
        for (auto &col : columns) {
            if (col == h[ii].name) {
                found = true;
                break;
            }
        }
        if (found == false) {
            col_pos.push_back(ii);
            out_headers.push_back(h[ii]);
        }
    }
    h = std::move(out_headers);
}

bool colsstmt::apply(models::row &row) {
    models::row ret;
    for (auto pos : col_pos) {
        ret.push_back(row[pos]);
    }
    row = std::move(ret);
    return true;
}

} // namespace engine
