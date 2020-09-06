#include "parser.hh"
#include "../engine/colsstmt.hh"
#include "../engine/engine.hh"
#include "../engine/selectstmt.hh"
#include "../engine/sortstmt.hh"
#include "../engine/to_num_stmt.hh"
#include "../engine/to_str_stmt.hh"
#include <fmt/format.h>
#include <tao/pegtl.hpp>
#include <tao/pegtl/analyze.hpp>
#include <vector>

namespace parser {

using namespace tao::pegtl;

struct comment : seq<two<'/'>, until<eolf>> {};
struct sp : sor<space, comment> {};
struct sps : star<sp> {};

struct comma : one<','> {};
struct dot : one<'.'> {};
struct equ : one<'='> {};
struct semi : one<';'> {};
struct bang : one<'!'> {};

// boolean operators
struct op_ne : string<'!', '='> {};
struct op_eq : two<'='> {};
struct op_gt : one<'>'> {};
struct op_lt : one<'<'> {};
struct op_gte : string<'>', '='> {};
struct op_lte : string<'<', '='> {};
struct op_and : two<'&'> {};
struct op_or : two<'|'> {};
struct op_assign : equ {};
struct op_regex : string<'=', '~'> {};
struct op_bool : sor<op_eq, op_ne, op_lte, op_gte, op_gt, op_lt, op_and, op_or,
                     op_regex, op_assign> {};

struct ident_first : ranges<'a', 'z', 'A', 'Z'> {};
struct ident_uscore : ranges<'a', 'z', 'A', 'Z', '_'> {};
struct ident_other : ranges<'a', 'z', 'A', 'Z', '0', '9', '_'> {};
struct ident : seq<ident_first, star<ident_other>> {};
struct full_ident : list_must<ident, dot> {};

struct uint_lit : plus<digit> {};
struct sign : one<'+', '-'> {};
struct int_lit : seq<opt<sign>, uint_lit> {};
struct float_lit : seq<int_lit, opt<seq<dot, uint_lit>>> {};
struct num_lit : sor<int_lit, float_lit> {};
struct char_escape : one<'a', 'b', 'f', 'n', 'r', 't', 'v', '\\', '\'', '"'> {};
struct escape : if_must<one<'\\'>, char_escape> {};
struct char_value : sor<escape, not_one<'\n', '\0'>> {};

template <char Q>
struct str_impl : if_must<one<Q>, until<one<Q>, char_value>> {};
struct str_lit : sor<str_impl<'\''>, str_impl<'"'>> {};

struct bool_lit
    : seq<sor<string<'t', 'r', 'u', 'e'>, string<'f', 'a', 'l', 's', 'e'>>,
          not_at<ident_other>> {};

struct constant : sor<num_lit, str_lit> {};

struct oparan : one<'('> {};
struct cparan : one<')'> {};
struct obrace : one<'{'> {};
struct cbrace : one<'}'> {};

template <typename Rule> struct action {};
template <typename Rule> struct control : normal<Rule> {};

// instructions
// to_num
struct to_num : string<'t', 'o', '_', 'n', 'u', 'm'> {};
struct to_num_stmt
    : seq<opt<bang>, to_num, oparan, list_must<ident, comma, sp>, cparan> {};

template <> struct control<to_num_stmt> : normal<to_num_stmt> {
    template <typename Input>
    static void start(const Input & /*unused*/, engine::engine &e) {
        e.new_stmt<engine::to_num_stmt>(); // TODO: fix
    }
    template <typename Input>
    static void success(const Input & /*unused*/, engine::engine &e) {
        e.finish_stmt();
    }
};

// to_str
struct to_str : string<'t', 'o', '_', 's', 't', 'r'> {};
struct to_str_stmt
    : seq<opt<bang>, to_str, oparan, list_must<ident, comma, sp>, cparan> {};

template <> struct control<to_str_stmt> : normal<to_str_stmt> {
    template <typename Input>
    static void start(const Input & /*unused*/, engine::engine &e) {
        e.new_stmt<engine::to_str_stmt>(); // TODO: fix
    }
    template <typename Input>
    static void success(const Input & /*unused*/, engine::engine &e) {
        e.finish_stmt();
    }
};

// cols
struct cols : string<'c', 'o', 'l', 's'> {};
struct colsstmt
    : seq<opt<bang>, cols, oparan, list_must<ident, comma, sp>, cparan> {};

template <> struct control<colsstmt> : normal<colsstmt> {
    template <typename Input>
    static void start(const Input & /*unused*/, engine::engine &e) {
        e.new_stmt<engine::colsstmt>();
    }
    template <typename Input>
    static void success(const Input & /*unused*/, engine::engine &e) {
        e.finish_stmt();
    }
};

// select
struct expr;
struct expr_group : if_must<oparan, expr, cparan> {};
struct expr_item : sor<ident, constant, expr_group> {};
struct expr : list<expr_item, op_bool, sp> {};
struct select : string<'s', 'e', 'l', 'e', 'c', 't'> {};
struct selectstmt : seq<select, oparan, expr, cparan> {};

template <> struct control<selectstmt> : normal<selectstmt> {
    template <typename Input>
    static void start(const Input & /*unused*/, engine::engine &e) {
        e.new_stmt<engine::selectstmt>();
    }
    template <typename Input>
    static void success(const Input & /*unused*/, engine::engine &e) {
        e.finish_stmt();
    }
};

template <> struct control<expr_group> : normal<expr_group> {
    template <typename Input>
    static void start(const Input & /*unused*/, engine::engine &e) {
        e.add_oper("(");
    }
    template <typename Input>
    static void success(const Input & /*unused*/, engine::engine &e) {
        e.add_oper(")");
    }
};

template <> struct action<op_bool> {
    template <typename Input>
    static void apply(const Input &in, engine::engine &e) {
        e.add_oper(in.string());
    }
};

// sort
struct op_reverse : one<'r'> {};
struct op_numeric : one<'n'> {};
struct op_sort : sor<op_reverse, op_numeric> {};
struct sortitem : seq<ident, opt<seq<one<':'>, op_sort, opt<op_sort>>>> {};
struct sort : string<'s', 'o', 'r', 't'> {};
struct sortstmt : seq<sort, oparan, list_must<sortitem, comma, sp>, cparan> {};

template <> struct control<sortstmt> : normal<sortstmt> {
    template <typename Input>
    static void start(const Input & /*unused*/, engine::engine &e) {
        e.new_stmt<engine::sortstmt>();
    }
    template <typename Input>
    static void success(const Input & /*unused*/, engine::engine &e) {
        e.finish_stmt();
    }
};

template <> struct action<op_sort> {
    template <typename Input>
    static void apply(const Input &in, engine::engine &e) {
        e.add_oper(in.string());
    }
};

// generic
struct anystmt : sor<colsstmt, selectstmt, to_num_stmt, to_str_stmt, sortstmt> {
};
struct block : seq<opt<sp>, list<anystmt, semi, sp>, opt<semi>> {};
struct thread_block : seq<obrace, sps, block, sps, cbrace, sps> {};
struct pgm : must<sor<block, star<thread_block>>, eof> {};

template <> struct action<ident> {
    template <typename Input>
    static void apply(const Input &in, engine::engine &e) {
        e.add_ident(in.string());
    }
};

template <> struct action<bang> {
    template <typename Input>
    static void apply(const Input & /*in*/, engine::engine &e) {
        e.add_bang();
    }
};

template <> struct action<str_lit> {
    template <typename Input>
    static void apply(const Input &in, engine::engine &e) {
        e.add_str(in.string());
    }
};

template <> struct action<num_lit> {
    template <typename Input>
    static void apply(const Input &in, engine::engine &e) {
        e.add_num(in.string());
    }
};

template <> struct action<block> {
    template <typename Input> static void apply(const Input &in) {}
};

template <> struct action<thread_block> {
    template <typename Input> static void apply(const Input &in) {}
};

template <> struct action<pgm> {
    template <typename Input> static void apply(const Input &in) {}
};

void run(const std::string &program, engine::engine &e) {
    if (analyze<pgm>() != 0) {
        throw std::runtime_error("analyze failed");
    }

    auto inp = string_input(program, "input");
    parse<pgm, action, control>(inp, e);
}

} // namespace parser
