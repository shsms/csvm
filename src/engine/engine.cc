#include "engine.hh"
#include "../models/models.hh"
#include "colsstmt.hh"
#include "selectstmt.hh"
#include <iostream>

namespace engine {

void engine::finish_stmt() {
    curr_stmt->finalize();
    curr_block.emplace_back(curr_stmt);
};

void engine::add_ident(const std::string &ident) {
    curr_stmt->add_ident(ident);
};

void engine::add_str(const std::string &str) { curr_stmt->add_str(str); };

void engine::add_num(const std::string &str) { curr_stmt->add_num(str); };

void engine::add_bang() { curr_stmt->add_bang(); };

void engine::add_oper(const std::string &oper) {
    curr_stmt->add_oper(oper);
}

bool engine::apply(models::row &row) {
    bool keep = true;
    for (auto &s : curr_block) {
        keep = s->apply(row);
        if (keep == false) {
            return false;
        }
    }
    return true;
}

std::string engine::string() {
    std::string ret;
    int ctr = 1;
    for (auto &s : curr_block) {
        ret += std::to_string(ctr++) + ". " + s->string();
    }
    return ret;
};

void engine::set_header(models::header_row &h) {
    for (auto &s : curr_block) {
        s->set_header(h);
    }
    std::string buffer{};
    for (auto ii = 0; ii < h.size(); ii++)
        if (ii == 0) // TODO compare with print first col outside loop
	    buffer += h[ii].name;
        else
	    buffer += "," + h[ii].name;
    buffer += "\n";
    std::cout << buffer;
}

bool engine::has_header() {
    return header_set;
}
} // namespace engine
