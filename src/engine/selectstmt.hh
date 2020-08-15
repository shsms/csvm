#include "stmt.hh"
#include "internal_select.hh"
#include <algorithm>
#include <stack>

namespace engine {

class selectstmt : public stmt {
    std::vector<std::shared_ptr<internal::select::token>> steps;
    std::stack<std::shared_ptr<internal::select::token>> stack;
    std::stack<models::value> eval_stack;
public:
    void add_ident(const std::string &ident) override;
    void add_str(const std::string &) override;
    void add_bang() override;
    void add_oper(const std::string &) override;
    void finalize() override;
    std::string string() override;
    models::row set_header(const models::row &h) override;
    bool apply(models::row &) override;
};
}
