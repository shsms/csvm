#include "expr.hh"
#include <iostream>
#include <string>

namespace engine::expr {

const std::unordered_map<std::string, int> expr::precedence = {
    {"(", -1}, {"!", 0},  {"*", 1},  {"/", 1},  {"/", 1},  {"+", 2},
    {"-", 2},  {"<", 3},  {">", 3},  {"<=", 3}, {">=", 3}, {"==", 4},
    {"=~", 4}, {"!=", 4}, {"&&", 5}, {"||", 6}, {")", 7},  {"=", 8},
};

void expr::add_ident(const std::string &val) {
    steps.emplace_back(
        std::static_pointer_cast<token>(std::make_shared<ident>(val)));
}

void expr::add_str(const std::string &val) {
    steps.emplace_back(std::static_pointer_cast<token>(
        std::make_shared<str>(val.substr(1, val.length() - 2))));
}

void expr::add_num(const std::string &str) {
    steps.emplace_back(
        std::static_pointer_cast<token>(std::make_shared<num>(str)));
}

void expr::add_bang() {
    steps.emplace_back(
        std::static_pointer_cast<token>(std::make_shared<not_oper>("!")));
}

void expr::add_oper(const std::string &oper) {
    if (!stack.empty() && oper != "(") {
        auto op_prec = precedence.find(oper)->second;
        for (auto a = stack.back();
             op_prec > precedence.find(a->string())->second;) {
            if (oper == ")" && a->string() == "(") {
                stack.pop_back();
                break;
            } else if (a->string() == "(") {
                break;
            }
            steps.emplace_back(a);
            stack.pop_back();
            if (stack.empty())
                break;
            a = stack.back();
        }
    }
    if (oper == "<") {
        stack.emplace_back(
            std::static_pointer_cast<token>(std::make_shared<lt_oper>(oper)));
    } else if (oper == ">") {
        stack.emplace_back(
            std::static_pointer_cast<token>(std::make_shared<gt_oper>(oper)));
    } else if (oper == "<=") {
        stack.emplace_back(
            std::static_pointer_cast<token>(std::make_shared<lte_oper>(oper)));
    } else if (oper == ">=") {
        stack.emplace_back(
            std::static_pointer_cast<token>(std::make_shared<gte_oper>(oper)));
    } else if (oper == "==") {
        stack.emplace_back(
            std::static_pointer_cast<token>(std::make_shared<eq_oper>(oper)));
    } else if (oper == "!=") {
        stack.emplace_back(
            std::static_pointer_cast<token>(std::make_shared<neq_oper>(oper)));
    } else if (oper == "&&") {
        stack.emplace_back(
            std::static_pointer_cast<token>(std::make_shared<and_oper>(oper)));
    } else if (oper == "||") {
        stack.emplace_back(
            std::static_pointer_cast<token>(std::make_shared<or_oper>(oper)));
    } else if (oper == "(") {
        stack.emplace_back(std::static_pointer_cast<token>(
            std::make_shared<oparan_oper>(oper)));
    } else if (oper == ")") {
        // do nothing here I guess
    }
}

void expr::finalize() {
    while (!stack.empty()) {
        steps.emplace_back(stack.back());
        stack.pop_back();
    }
}
std::string expr::string() {
    std::string ret;
    for (auto step : steps) {
        ret += step->string() + " ";
    }
    ret += "\n";
    for (auto step : stack) {
        ret += step->string() + " ";
    }
    ret += "\n\n";
    return ret;
}

models::header_row expr::set_header(const models::header_row &h) {
    for (auto &step : steps) {
        step->set_header(h);
    }
    return h;
}

bool expr::apply(models::row &row) {
    for (auto step : steps) {
        step->apply(row, eval_stack);
    }
    if (auto [sel, ok] = models::get_bool_value(eval_stack.top()); sel && ok) {
        return true;
    }
    return false;
}

} // namespace engine::expr