#include "csv.hh"
#include "../order.hh"
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
    for (auto ii = 0; ii < curr_row.size(); ii++)
        if (ii == 0)
            print_buffer += std::get<std::string>(curr_row[ii]);
        else
            print_buffer += comma_str + std::get<std::string>(curr_row[ii]);
    print_buffer += newline;

    curr_row.clear();
}

void csv::print() noexcept {
    if (print_buffer.length() > 0)
        std::cout << print_buffer;
}

void parse_body(engine::engine &e, std::string &&data, int token,
                threading::ordering_lock &lock) {
    // if (analyze<file>() != 0) {
    //     fmt::print("analyze failed");
    // } else {
    //     fmt::print("analyze success\n");
    // }
    csv csv(e, data.size());
    string_input in(std::move(data), "csv");
    parse<file, action>(in, csv);
    if (token >= 0) {
        lock.lock(token);
        csv.print();
        lock.unlock();
    } else {
        csv.print();
    }
}

void parse_header(engine::engine &e, std::string &&h) {
    csv csv(e, h.size());
    string_input in(std::move(h), "header");
    parse<header_line, action>(in, csv);
}
} // namespace csv
