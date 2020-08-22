#ifndef CSVQ_EXPR_H
#define CSVQ_EXPR_H

#include "expr_tokens.hh"
#include "stmt.hh"
#include <stack>
#include <string>

namespace engine::expr {

class expr : public stmt {
    std::vector<std::shared_ptr<token>> steps;
    std::deque<std::shared_ptr<token>> stack;

    static const std::unordered_map<std::string, int> precedence;

  public:
    void add_ident(const std::string &ident) override;
    void add_str(const std::string &) override;
    void add_num(const std::string &) override;
    void add_bang() override;
    void add_oper(const std::string &) override;
    void finalize() override;
    std::string string() override;
    void set_header(models::header_row &h) override;
    bool apply(models::row &, std::stack<models::value>& eval_stack) const override;
};

} // namespace engine::expr

#endif
