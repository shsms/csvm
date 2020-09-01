#include "../engine/engine.hh"
#include "../models/models.hh"
#include "../order.hh"
#include "../queue.hh"
#include <stack>
#include <string>

namespace csv {

class csv {
    models::row curr_row;
    models::bin_chunk processed;

  public:
    csv(int id) :processed({.id = id}) { }
    void new_row();
    void add_value(std::string &&);
    models::bin_chunk get();
};

// TODO: make engine& a const ref after migrating parse_body
models::bin_chunk parse_body(models::raw_chunk &&);
models::header_row parse_header(std::string &&);
} // namespace csv
