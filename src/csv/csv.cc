#include "csv.hh"
#include <fmt/format.h>
#include <tao/pegtl.hpp>
#include <tao/pegtl/analyze.hpp>

namespace csv {
using namespace tao::pegtl;

template <char C> struct string_without : star<not_one<C, 10, 13>> {};
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
    template <typename Input> static void apply(const Input &in, csv &parser) {
        parser.header.push_back(in.string());
    }
};

template <> struct action<header_line> {
    template <typename Input> static void apply(const Input &in, csv &parser) {
        parser.set_header();
    }
};

template <> struct action<value> {
    template <typename Input> static void apply(const Input &in, csv &parser) {
        parser.add_value(in.string());
    }
};

template <> struct action<line> {
    template <typename Input> static void apply(const Input &in, csv &parser) {
        parser.new_row();
    }
};

void run(const std::string &csvfile, engine::engine &e) {
    // if (analyze<file>() != 0) {
    //     fmt::print("analyze failed");
    // } else {
    //     fmt::print("analyze success\n");
    // }
    csv parser = {.e = e};
    file_input in(csvfile);
    parse<hfile, action>(in, parser);
}

void csv::add_value(const std::string &v) { curr_row.push_back(v); }
void csv::set_header() { e.set_header(header); }
void csv::new_row() {
    e.apply(curr_row);
    curr_row.clear();
}
} // namespace csv
