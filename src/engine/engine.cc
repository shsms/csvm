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

void engine::apply(models::row &row) {
    bool keep = true;
    for (auto &s : curr_block) {
        keep = s->apply(row);
        if (keep == false) {
            return;
        }
    }
    static const std::string comma_str = ",";
    static const std::string newline = "\n";
    if (print_buffer.length() >= 1e4) {
        std::cout << print_buffer;
        print_buffer.clear();
    }
    for (auto ii = 0; ii < row.size(); ii++)
        if (ii == 0)
            print_buffer += std::get<std::string>(row[ii]);
        else
            print_buffer += comma_str + std::get<std::string>(row[ii]);
    print_buffer += newline;
}

std::string engine::string() {
    std::string ret;
    int ctr = 1;
    for (auto &s : curr_block) {
        ret += std::to_string(ctr++) + ". " + s->string();
    }
    return ret;
};

void engine::set_header(const models::header_row &h) {
    auto nextrow = h;
    for (auto &s : curr_block) {
        nextrow = s->set_header(nextrow);
    }
    std::string buffer{};
    for (auto ii = 0; ii < nextrow.size(); ii++)
        if (ii == 0) // TODO compare with print first col outside loop
	    buffer += nextrow[ii].name;
        else
	    buffer += "," + nextrow[ii].name;
    buffer += "\n";
    std::cout << buffer;
}

void engine::cleanup() {
    if (print_buffer.length() > 0)
        std::cout << print_buffer;
}

} // namespace engine
