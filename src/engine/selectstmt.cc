#include "selectstmt.hh"

namespace engine {
    void selectstmt::add_ident(const std::string &ident) {
	steps.emplace_back(std::static_pointer_cast<internal::select::token>(
			       std::make_shared<internal::select::ident>(ident)));
    }

    void selectstmt::add_str(const std::string &str) {
	steps.emplace_back(std::static_pointer_cast<internal::select::token>(
			       std::make_shared<internal::select::str>(str.substr(1, str.length()-2))));
    }

    void selectstmt::add_bang() {
	steps.emplace_back(
	    std::static_pointer_cast<internal::select::token>(
		std::make_shared<internal::select::not_oper>()));
    }

    void selectstmt::add_oper(const std::string &oper) {
	if (oper == "==") {
	    stack.emplace(
		std::static_pointer_cast<internal::select::token>(
		    std::make_shared<internal::select::eq_oper>()));
	}
    }

    void selectstmt::finalize() {
	while (!stack.empty()) {
	    steps.push_back(stack.top());
	    stack.pop();
	}
    }
    std::string selectstmt::string() {
	return "";
    }

    models::row selectstmt::set_header(const models::row &h) {
	for (auto &step : steps) {
	    step->set_header(h);
	}
	return h;
    }

    models::row selectstmt::apply(const models::row &row) {
	for (auto step : steps) {
	    step->apply(row, eval_stack);
	}
	if (auto [sel,ok] = models::get_bool_value(eval_stack.top()); sel&&ok) {
	    return row;
	}
	return models::row();
    }

    
}
