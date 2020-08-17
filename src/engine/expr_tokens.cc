#include "expr_tokens.hh"

namespace engine::expr {

ident::ident(const std::string &s) : token(s) { value = s; }

void ident::apply(const models::row &row,
                  std::stack<models::value> &eval_stack) {
    eval_stack.emplace(row[col_pos]);
}

void ident::set_header(const models::header_row &h) {
    bool found = false;
    for (auto ii = 0; ii < h.size(); ii++) {
        if (value == h[ii].name) {
            col_pos = ii;
            found = true;
	    return;
        }
    }
    if (found == false) {
        throw std::runtime_error("field not found:" + value);
    }
}

str::str(const std::string &s) : token(s) { value = s; }

void str::apply(const models::row &row, std::stack<models::value> &eval_stack) {
    eval_stack.emplace(value);
}

num::num(const std::string &s) : token(s) { value = std::stod(s); }

num::num(const double &d) : token(std::to_string(d)) { value = d; }

void num::apply(const models::row &row, std::stack<models::value> &eval_stack) {
    eval_stack.emplace(value);

}

void not_oper::apply(const models::row &row,
                     std::stack<models::value> &eval_stack) {
    auto [op, ok] = models::get_bool_value(eval_stack.top());
    eval_stack.pop();
    eval_stack.emplace(!op);
}

void gte_oper::apply(const models::row &row,
                     std::stack<models::value> &eval_stack) {
    auto op2 = eval_stack.top();
    eval_stack.pop();
    auto op1 = eval_stack.top();
    eval_stack.pop();
    eval_stack.emplace(!models::value_lt(op1, op2));
}

void lte_oper::apply(const models::row &row,
                     std::stack<models::value> &eval_stack) {
    auto op2 = eval_stack.top();
    eval_stack.pop();
    auto op1 = eval_stack.top();
    eval_stack.pop();
    eval_stack.emplace(!models::value_gt(op1, op2));
}

void gt_oper::apply(const models::row &row,
                    std::stack<models::value> &eval_stack) {
    auto op2 = eval_stack.top();
    eval_stack.pop();
    auto op1 = eval_stack.top();
    eval_stack.pop();
    eval_stack.emplace(models::value_gt(op1, op2));
}

void lt_oper::apply(const models::row &row,
                    std::stack<models::value> &eval_stack) {
    auto op2 = eval_stack.top();
    eval_stack.pop();
    auto op1 = eval_stack.top();
    eval_stack.pop();
    eval_stack.emplace(models::value_lt(op1, op2));
}

void eq_oper::apply(const models::row &row,
                    std::stack<models::value> &eval_stack) {
    // TODO: ref vs value
    auto op2 = eval_stack.top();
    eval_stack.pop();
    auto op1 = eval_stack.top();
    eval_stack.pop();
    eval_stack.emplace(models::value_equal(op1, op2));
}

void regex_oper::apply(const models::row &row,
                       std::stack<models::value> &eval_stack) {
    // TODO
}

void neq_oper::apply(const models::row &row,
                     std::stack<models::value> &eval_stack) {
    auto op2 = eval_stack.top();
    eval_stack.pop();
    auto op1 = eval_stack.top();
    eval_stack.pop();
    eval_stack.emplace(!models::value_equal(op1, op2));
}

void and_oper::apply(const models::row &row,
                     std::stack<models::value> &eval_stack) {
    auto [op2, ok2] = models::get_bool_value(eval_stack.top());
    eval_stack.pop();
    auto [op1, ok1] = models::get_bool_value(eval_stack.top());
    eval_stack.pop();
    // if (!ok1 || !ok2) { // TODO: replace with engine analyser
    //     throw std::runtime_error("&& not ok");
    // }
    eval_stack.emplace(op1 && op2);
}

void or_oper::apply(const models::row &row,
                    std::stack<models::value> &eval_stack) {
    auto [op2, ok2] = models::get_bool_value(eval_stack.top());
    eval_stack.pop();
    auto [op1, ok1] = models::get_bool_value(eval_stack.top());
    eval_stack.pop();
    eval_stack.emplace(op1 || op2);
}
} // namespace engine::expr
