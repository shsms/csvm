#include "parser.hh"
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
    struct op_eq : string<'=', '='> {};
    struct op_gt : one<'>'> {};
    struct op_lt : one<'<'> {};
    struct op_gte : string<'>', '='> {};
    struct op_lte : string<'<', '='> {};
    struct op_bool : sor<op_eq, op_ne, op_gt, op_lt, op_lte, op_gte>{};

    struct op_and : two<'&'>{};
    struct op_or : two<'|'>{};
    struct op_and_or : sor<op_and, op_or>{};
    
    struct ident_first : ranges<'a', 'z', 'A', 'Z'> {};
    struct ident_other : ranges<'a', 'z', 'A', 'Z', '0', '9', '_'> {};
    struct ident : seq<ident_first, star<ident_other>> {};
    struct full_ident : list_must<ident, dot> {};

    struct int_lit : seq<range<'1', '9'>, star<digit>> {};
    struct char_escape : one<'a', 'b', 'f', 'n', 'r', 't', 'v', '\\', '\'', '"'> {};
    struct escape : if_must<one<'\\'>, char_escape> {};
    struct char_value : sor<escape, not_one<'\n', '\0'>> {};

    template <char Q>
    struct str_impl : if_must<one<Q>, until<one<Q>, char_value>> {};
    struct str_lit : sor<str_impl<'\''>, str_impl<'"'>> {};

    struct bool_lit
	: seq<sor<string<'t', 'r', 'u', 'e'>, string<'f', 'a', 'l', 's', 'e'>>,
	      not_at<ident_other>> {};

    struct sign : one<'+', '-'> {};
    struct constant : sor<bool_lit, seq<opt<sign>, int_lit>, str_lit> {
    };

    struct oparan : one<'('> {};
    struct cparan : one<')'> {};
    struct obrace : one<'{'> {};
    struct cbrace : one<'}'> {};

    // instructions
    // cols
    struct cols : string<'c', 'o', 'l', 's'> {};
    struct colsstmt
	: seq<opt<bang>, cols, oparan, list_must<ident, comma, sp>, cparan> {};

    // select
    struct bool_expr_1 : seq<ident, opt<sp>, op_bool, opt<sp>, sor<ident, constant>>{};
    struct str_compare_expr_1 : seq<opt<bang>, ident, dot, ident, oparan, str_lit, cparan>{};
    struct any_bool_expr_1 : sor<bool_expr_1, str_compare_expr_1>{};
    struct bool_expr_multi_noparan : list_must<any_bool_expr_1, op_and_or, sp>{};
    struct bool_expr_multi_paran : seq<oparan, sp, bool_expr_multi_noparan, cparan>{};
    struct bool_expr_multi : sor<bool_expr_multi_paran, bool_expr_multi_noparan>{};
    struct bool_expr : list_must<bool_expr_multi, op_and_or, sp>{};
    struct select : string<'s','e','l','e','c','t'>{};
    struct selectstmt : seq<select, oparan, bool_expr, cparan>{};
    
    // generic    
    struct anystmt : sor<colsstmt, selectstmt> {};
    struct block : list<anystmt, semi, sp> {};
    struct thread_block : seq<obrace, sps, block, sps, cbrace, sps> {};
    struct pgm : must<sor<block, star<thread_block>>, eof> {};

    template <typename Rule> struct action {};

    template <> struct action<cols> {
	template <typename Input>
	static void apply(const Input &in) {
	    fmt::print("cols: {}\n", in.string());
	}
    };

    template <> struct action<select> {
	template <typename Input>
	static void apply(const Input &in) {
	    fmt::print("select: {}\n", in.string());
	}
    };

    template <> struct action<ident> {
	template <typename Input> static void apply(const Input &in) {
	    fmt::print("ident: {}\n", in.string());
	}
    };

    template <> struct action<bang> {
	template <typename Input> static void apply(const Input &in) {
	    fmt::print("bang: {}\n", in.string());
	}
    };

    template <> struct action<colsstmt> {
	template <typename Input> static void apply(const Input &in) {
	    fmt::print("colsstmt: {}\n", in.string());
	}
    };

    template <> struct action<block> {
	template <typename Input> static void apply(const Input &in) {
	    fmt::print("block: {}\n", in.string());
	}
    };

    template <> struct action<thread_block> {
	template <typename Input> static void apply(const Input &in) {
	    fmt::print("thread_block: {}\n", in.string());
	}
    };

    template <> struct action<pgm> {
	template <typename Input> static void apply(const Input &in) {
	    fmt::print("pgm: {}\n", in.string());
	}
    };

    void run(const std::string &program) {
	if (analyze<pgm>() != 0) {
	    fmt::print("analyze failed");
	} else {
	    fmt::print("analyze success\n");
	}

	auto inp = string_input(program, "input");
	parse<pgm, action>(inp);
    }

} // namespace parser
