#include "../engine/engine.hh"
#include "../models/models.hh"
#include "../order.hh"
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

    csv(engine::engine &e, size_t inp_size):e{e} { print_buffer.reserve(inp_size); }
    void set_header();
    void add_value(std::string&&);
    void new_row();
    void print() noexcept;
};

// TODO: make engine& a const ref after migrating parse_body
void parse_body(engine::engine &, std::string &&, int, threading::ordering_lock &);
void parse_header(engine::engine &, std::string &&);
} // namespace csv
