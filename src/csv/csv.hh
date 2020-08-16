#include "../engine/engine.hh"
#include "../models/models.hh"
#include <string>
namespace csv {

struct csv {
    engine::engine &e;
    models::header_row header;
    models::row curr_row;
    void set_header();
    void add_value(std::string&&);
    void new_row();
};

void run(const std::string &, engine::engine &e);
} // namespace csv
