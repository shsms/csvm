#ifndef CSVM_EXPR_TOKENS_H
#define CSVM_EXPR_TOKENS_H

#include "../models/models.hh"
#include <stack>
#include <string>
#include <utility>

namespace engine::expr {
class token {
    const std::string str_repr;

  public:
    token(std::string s) : str_repr(std::move(s)) {}
    virtual ~token() {}
    virtual void apply(const models::row &, std::stack<models::value> &) const = 0;
    virtual void set_header(const models::header_row & /*unused*/){};
    virtual const std::string &string() { return str_repr; };
};

class ident : public token {
    std::string value;
    int col_pos{};

  public:
    ident(const std::string &); // {        value = s;    }
    void apply(const models::row & /*unused*/,
               std::stack<models::value> & /*unused*/) const override;
    void set_header(const models::header_row & /*unused*/) override;
};

class str : public token {
    std::string value;

  public:
    str(const std::string &); // {        value = s;    }
    void apply(const models::row & /*unused*/,
               std::stack<models::value> & /*unused*/) const override;
    const std::string &string() override { return value; }
};

class num : public token {
    double value;

  public:
    num(const std::string &); // {        value = s;    }
    num(const double &);      // {        value = s;    }

    void apply(const models::row & /*unused*/,
               std::stack<models::value> & /*unused*/) const override;
};

class not_oper : public token {
  public:
    not_oper(const std::string &t) : token(t) {}
    void apply(const models::row & /*unused*/,
               std::stack<models::value> & /*unused*/) const override;
};

class gte_oper : public token {
  public:
    gte_oper(const std::string &t) : token(t) {}
    void apply(const models::row & /*unused*/,
               std::stack<models::value> & /*unused*/) const override;
};

class lte_oper : public token {
  public:
    lte_oper(const std::string &t) : token(t) {}
    void apply(const models::row & /*unused*/,
               std::stack<models::value> & /*unused*/) const override;
};

class gt_oper : public token {
  public:
    gt_oper(const std::string &t) : token(t) {}
    void apply(const models::row & /*unused*/,
               std::stack<models::value> & /*unused*/) const override;
};

class lt_oper : public token {
  public:
    lt_oper(const std::string &t) : token(t) {}
    void apply(const models::row & /*unused*/,
               std::stack<models::value> & /*unused*/) const override;
};

class eq_oper : public token {
  public:
    eq_oper(const std::string &t) : token(t) {}
    void apply(const models::row & /*unused*/,
               std::stack<models::value> & /*unused*/) const override;
};

class regex_oper : public token {
  public:
    regex_oper(const std::string &t) : token(t) {}
    void apply(const models::row & /*unused*/,
               std::stack<models::value> & /*unused*/) const override;
};

class neq_oper : public token {
  public:
    neq_oper(const std::string &t) : token(t) {}
    void apply(const models::row & /*unused*/,
               std::stack<models::value> & /*unused*/) const override;
};

class and_oper : public token {
  public:
    and_oper(const std::string &t) : token(t) {}
    void apply(const models::row & /*unused*/,
               std::stack<models::value> & /*unused*/) const override;
};

class or_oper : public token {
  public:
    or_oper(const std::string &t) : token(t) {}
    void apply(const models::row & /*unused*/,
               std::stack<models::value> & /*unused*/) const override;
};

class oparan_oper : public token {
  public:
    oparan_oper(const std::string &t) : token(t) {}
    // needed only while parsing, so empty implementation.
    void apply(const models::row & /*unused*/,
               std::stack<models::value> & /*unused*/) const override{};
};

} // namespace engine::expr

#endif
