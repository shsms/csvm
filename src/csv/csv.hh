#include "../engine/engine.hh"
#include "../models/models.hh"
#include <string>
namespace csv {

struct csv {
    engine::engine &e;
    models::row header;
    models::row curr_row;
    void set_header();
    void add_value(const std::string &);
    void new_row();
};

void run(const std::string &, engine::engine &e);
} // namespace csv
