#include "engine.hh"
#include "../models/models.hh"
#include "colsstmt.hh"

namespace engine {
void engine::new_cols_stmt() {
    curr_stmt = std::static_pointer_cast<stmt>(std::make_shared<colsstmt>());
};

void engine::finish_stmt() { curr_block.emplace_back(curr_stmt); };

void engine::add_ident(const std::string &ident) {
    curr_stmt->add_ident(ident);
};

void engine::add_bang() { curr_stmt->add_bang(); };

void engine::apply(models::row &row) {
    auto nextrow = row;
    for (auto &s : curr_block) {
        nextrow = s->apply(nextrow);
    }
    for (auto ii = 0; ii < nextrow.size(); ii++)
        if (ii == 0) // TODO compare with print first col outside loop
	    models::print("{}", nextrow[ii]);
        else
	    models::print(",{}", nextrow[ii]);
    fmt::print("\n");
}

std::string engine::string() {
    std::string ret;
    int ctr = 1;
    for (auto &s : curr_block) {
        ret += std::to_string(ctr++) + ". " + s->string();
    }
    return ret;
};

void engine::set_header(const models::row &h) {
    auto nextrow = h;
    for (auto &s : curr_block) {
        nextrow = s->set_header(nextrow);
    }
    for (auto ii = 0; ii < nextrow.size(); ii++)
        if (ii == 0) // TODO compare with print first col outside loop
            models::print("{}", nextrow[ii]);
        else
            models::print(",{}", nextrow[ii]);
    fmt::print("\n");
}

} // namespace engine