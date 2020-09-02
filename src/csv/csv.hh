#include "../engine/engine.hh"
#include "../models/models.hh"
#include "../order.hh"
#include "../queue.hh"
#include <stack>
#include <string>

namespace csv {

class csv {
    int curr_row{};
    models::bin_chunk &parsed;

  public:
    csv(models::bin_chunk &p) :parsed(p) {
	if (parsed.data.size() == 0) {
	    parsed.data.emplace_back(models::row{});
	} else {
	    parsed.data[0].first.clear();
	}
    }
    void new_row();
    void add_value(std::string &&);
    inline int get_length() { return curr_row; }
};

// TODO: make engine& a const ref after migrating parse_body
void parse_body(models::raw_chunk &&, models::bin_chunk &);
models::header_row parse_header(std::string &&);
} // namespace csv
