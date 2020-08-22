#include "csv.hh"
#include <fmt/format.h>
#include <tao/pegtl.hpp>
#include <tao/pegtl/analyze.hpp>
#include <stack>

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
    template <typename Input> static void apply(const Input &in, csv &csv) {
        csv.header.emplace_back(
            models::col_header{.name = in.string(), .type = models::string_t});
    }
};

template <> struct action<header_line> {
    template <typename Input> static void apply(const Input &in, csv &csv) {
        csv.set_header();
    }
};

template <> struct action<value> {
    template <typename Input> static void apply(const Input &in, csv &csv) {
        csv.add_value(std::move(in.string()));
    }
};

template <> struct action<line> {
    template <typename Input> static void apply(const Input &in, csv &csv) {
        csv.new_row();
    }
};

inline void csv::add_value(std::string &&v) { curr_row.emplace_back(v); }

inline void csv::set_header() { e.set_header(header); }

void csv::new_row() {
    if (e.apply(curr_row, eval_stack) == false) {
        curr_row.clear();
        return;
    }

    static const std::string comma_str = ",";
    static const std::string newline = "\n";
    if (print_buffer.length() >= 1e4) {
        std::cout << print_buffer;
        print_buffer.clear();
    }
    for (auto ii = 0; ii < curr_row.size(); ii++)
        if (ii == 0)
            print_buffer += std::get<std::string>(curr_row[ii]);
        else
            print_buffer += comma_str + std::get<std::string>(curr_row[ii]);
    print_buffer += newline;

    curr_row.clear();
}

void csv::cleanup() {
    if (print_buffer.length() > 0)
        std::cout << print_buffer;
}

void run(const std::string &csvfile, engine::engine &e) {
    // if (analyze<file>() != 0) {
    //     fmt::print("analyze failed");
    // } else {
    //     fmt::print("analyze success\n");
    // }
    csv csv(e);
    file_input in(csvfile);
    parse<hfile, action>(in, csv);
    csv.cleanup();
}

} // namespace csv
