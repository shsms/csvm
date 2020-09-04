#ifndef CSVM_CSV_HH
#define CSVM_CSV_HM

#include "../engine/engine.hh"
#include "../models/models.hh"
#include "../order.hh"
#include "../queue.hh"
#include <stack>
#include <string>
#include <tao/pegtl.hpp>

namespace csv {

using namespace tao::pegtl;

template <typename F> class csv {
    models::row curr_row{};
    F proc_fn;

  public:
    csv(F fn) : proc_fn(fn) {}

    inline void add_value(std::string &&v) {
        curr_row.emplace_back(std::move(v));
    }

    inline void new_row() {
        proc_fn(curr_row);
        curr_row.clear();
    }
};

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
    inline static void apply(const Input &in, models::header_row &header) {
        header.emplace_back(models::col_header{.name = in.string()});
    }
};

template <> struct action<value> {
    template <typename Input, typename F>
    inline static void apply(const Input &in, csv<F> &csv) {
        csv.add_value(std::move(in.string()));
    }
};

template <> struct action<line> {
    template <typename Input, typename F>
    inline static void apply(const Input & /*in*/, csv<F> &csv) {
        csv.new_row();
    }
};

template <typename F>
inline void parse_body(models::raw_chunk &&chunk, F proc_fn) {
    csv csv(proc_fn);
    tao::pegtl::string_input in(std::move(chunk.data), "csv");
    parse<file, action>(in, csv);
}

inline models::header_row parse_header(std::string &&h) {
    models::header_row header;
    string_input in(std::move(h), "header");
    parse<header_line, action>(in, header);
    return header;
}

} // namespace csv

#endif /* CSVM_CSV_HH */
