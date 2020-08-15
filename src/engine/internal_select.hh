#include "../models/models.hh"
#include <string>
#include <stack>

namespace engine::internal::select {
    
class token {
public:
    virtual void apply(const models::row&, std::stack<models::value>&) = 0;
    virtual void set_header(const models::row &) = 0;
};

class ident : public token {
    std::string value;
    int col_pos;
public:
    ident(const std::string&);  // {        value = s;    }
    void apply(const models::row&, std::stack<models::value>&) override;
    void set_header(const models::row&) override;
    
};

class str : public token {
    std::string value;
public:
    str(const std::string&);  // {        value = s;    }
    void apply(const models::row&, std::stack<models::value>&) override;
    void set_header(const models::row&) override {}
    
};

class not_oper : public token {
public:
    void apply(const models::row&, std::stack<models::value>&) override;
    void set_header(const models::row&) override {};
};

class gte_oper : public token {
public:
    void apply(const models::row&, std::stack<models::value>&) override;
    void set_header(const models::row&) override {};
};

class lte_oper : public token {
public:
    void apply(const models::row&, std::stack<models::value>&) override;
    void set_header(const models::row&) override {};
};

class gt_oper : public token {
public:
    void apply(const models::row&, std::stack<models::value>&) override;
    void set_header(const models::row&) override {};
};

class lt_oper : public token {
public:
    void apply(const models::row&, std::stack<models::value>&) override;
    void set_header(const models::row&) override {};
};

class eq_oper : public token {
public:
    void apply(const models::row&, std::stack<models::value>&) override;
    void set_header(const models::row&) override {};
};

class neq_oper : public token {
public:
    void apply(const models::row&, std::stack<models::value>&) override;
    void set_header(const models::row&) override {};
};
}
