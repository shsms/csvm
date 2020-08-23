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

    csv(engine::engine &e):e{e} {}
    void set_header();
    void add_value(std::string&&);
    void new_row();
    void cleanup();
};

// TODO: make engine& a const ref after migrating parse_body
void parse_body(engine::engine &, std::string &&, int );
void parse_header(engine::engine &, std::string &&);
} // namespace csv
