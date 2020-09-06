#include "sortstmt.hh"
#include <iostream>

namespace engine {

void sortstmt::add_ident(const std::string &col) {
    columns.emplace_back(colspec{false, false, 0, col});
    ++curr_pos;
}

void sortstmt::add_oper(const std::string &oper) {
    if (oper == "r") {
        columns[curr_pos - 1].descending = true;
    } else if (oper == "n") {
        columns[curr_pos - 1].numeric = true;
    } else {
        throw std::runtime_error(std::string("unknown operator to sort: ") +
                                 oper);
    }
}

void sortstmt::set_header(models::header_row &h) {
    for (auto &col : columns) {
        bool found = false;
        for (auto ii = 0; ii < h.size(); ++ii) {
            if (col.name == h[ii].name) {
                col.pos = ii;
                found = true;
                break;
            }
        }
        if (!found) {
            throw std::runtime_error("column not found in header:" + col.name);
        }
    }
    return;
}

std::string sortstmt::string() { return ""; }

bool sortstmt::apply(models::bin_chunk &chunk,
                     std::stack<models::value> &eval_stack) {
    std::stable_sort(chunk.data.begin(), chunk.data.end(),
                     [this](const models::row &a, const models::row &b) {
                         for (auto &col : this->columns) {
                             if (a[col.pos] > b[col.pos]) {
                                 return col.descending;
                             }
                             if (a[col.pos] < b[col.pos]) {
                                 return !col.descending;
                             }
                         }
                         return false;
                     });
    return true; // don't want print stuff to pick this up.
}

} // namespace engine
