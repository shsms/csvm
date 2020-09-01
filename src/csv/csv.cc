#include "csv.hh"
#include "../order.hh"
#include "../queue.hh"
#include <fmt/format.h>
#include <stack>
#include <tao/pegtl.hpp>
#include <tao/pegtl/analyze.hpp>

namespace csv {
using namespace tao::pegtl;

const int TABCHAR = 10, RETCHAR = 13;

template <char C> struct string_without : star<not_one<C, TABCHAR, RETCHAR>> {};
struct plain_value : string_without<','> {};
struct quoted_value : if_must<one<'"'>, string_without<'"'>, one<'"'>> {};
struct value : sor<quoted_value, plain_value> {};

struct header_value : sor<quoted_value, plain_value> {};
struct header_list : list_must<header_value, one<','>> {};
struct header_line : if_must<header_list, eolf> {};

struct value_list : list<value, one<','>> {};
struct line : seq<value_list, sor<eolf, eof>> {};

struct file : until<eof, line> {};
struct hfile : seq<header_line, file> {};

template <typename Rule> struct action {};

template <> struct action<header_value> {
    template <typename Input>
    static void apply(const Input &in, models::header_row &header) {
        header.emplace_back(
            models::col_header{.name = in.string(), .type = models::string_t});
    }
};

template <> struct action<value> {
    template <typename Input> static void apply(const Input &in, csv &csv) {
        csv.add_value(std::move(in.string()));
    }
};

template <> struct action<line> {
    template <typename Input>
    static void apply(const Input & /*in*/, csv &csv) {
        csv.new_row();
    }
};

inline void csv::add_value(std::string &&v) { curr_row.emplace_back(v); }

inline models::bin_chunk csv::get() { return std::move(processed); }

void csv::new_row() {
    processed.data.emplace_back(std::move(curr_row));
    // curr_row.clear();
}

models::bin_chunk parse_body(models::raw_chunk &&chunk) {
    // if (analyze<file>() != 0) {
    //     fmt::print("analyze failed");
    // } else {
    //     fmt::print("analyze success\n");
    // }
    csv csv(chunk.id);
    string_input in(std::move(chunk.data), "csv");
    parse<file, action>(in, csv);
    return csv.get();
}

models::header_row parse_header(std::string &&h) {
    models::header_row header;

    string_input in(std::move(h), "header");
    parse<header_line, action>(in, header);

    return header;
}
} // namespace csv
