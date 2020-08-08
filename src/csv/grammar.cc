#include <tao/pegtl.hpp>
#include <tao/pegtl/analyze.hpp>
#include <fmt/format.h>

namespace csv_grammar {
    using namespace tao::pegtl;
    
    template <char C> struct string_without : star<not_one<C, 10, 13>> {};
    struct plain_value : string_without<','> {};
    struct quoted_value : if_must<one<'"'>, string_without<'"'>, one<'"'>> {};
    struct value : sor<quoted_value, plain_value> {};
    
    struct header_value : sor<quoted_value, plain_value> {};
    struct header_list : list_must<header_value, one<','>> {};
    struct header_line : if_must<header_list, eolf> {};
    
    struct value_list : list_must<value, one<','>> {};
    struct line : if_must<value_list, eolf> {};

    struct hfile : seq<header_line, until< eof, line> > {};
    struct file : until< eof, line > {};

    template <typename Rule> struct action {};
    
    template <> struct action<value> {
	template <typename Input>
	static void apply(const Input &in) {
	}
    };
    void run(const std::string &program) {
	if (analyze<hfile>() != 0) {
	    fmt::print("analyze failed");
	} else {
	    fmt::print("analyze success\n");
	}

	auto inp = string_input(program, "input");
	parse<hfile, action>(inp);
    }

} // namespace csv
