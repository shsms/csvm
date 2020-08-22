#include "../engine/engine.hh"
#include "../models/models.hh"
#include <string>
#include <stack>

namespace csv {

class csv {
    engine::engine &e;
    std::string print_buffer;
    std::stack<models::value> eval_stack;

public:
    models::header_row header;
    models::row curr_row;
    uint64_t in_rows;

    csv(engine::engine &e):e{e}, in_rows{0} {}
    void set_header();
    void add_value(std::string&&);
    void new_row();
    void cleanup();
};

void run(const std::string &, engine::engine &e);
} // namespace csv
