#include "internal_select.hh"

namespace engine::internal::select {
    ident::ident(const std::string& s) {
	value = s;
    }
    void ident::apply(const models::row &row, std::stack<models::value> &eval_stack) {
	eval_stack.emplace(row[col_pos]);
    }
    void ident::set_header(const models::row& h) {
	bool found = false;
	for (auto ii = 0; ii < h.size(); ii++) {
	    if (value == h[ii].string_v) {
		col_pos = ii;
		found = true;
	    }
	}
	if (found == false) {
	    throw std::runtime_error("field not found:" + value);
	}
    }

    str::str(const std::string& s) {
	value = s;
    }
    
    void str::apply(const models::row &row, std::stack<models::value> &eval_stack) {
	eval_stack.emplace(models::value{.type=models::string_t,.string_v=value});
    }

    void not_oper::apply(const models::row &row, std::stack<models::value> &eval_stack) {
	// TODO
    }

    void gte_oper::apply(const models::row &row, std::stack<models::value> &eval_stack) {
	// TODO
    }

    void lte_oper::apply(const models::row &row, std::stack<models::value> &eval_stack) {
	// TODO
    }

    void gt_oper::apply(const models::row &row, std::stack<models::value> &eval_stack) {
	// TODO
    }

    void lt_oper::apply(const models::row &row, std::stack<models::value> &eval_stack) {
	// TODO
    }

    void eq_oper::apply(const models::row &row, std::stack<models::value> &eval_stack) {
	// TODO: ref vs value
	auto op2 = eval_stack.top();
	eval_stack.pop();
	auto op1 = eval_stack.top();
	eval_stack.pop();
	eval_stack.emplace(models::value{.type=models::bool_t, .bool_v=op1.string_v==op2.string_v});
    }

    void neq_oper::apply(const models::row &row, std::stack<models::value> &eval_stack) {
	// TODO
    }

}
