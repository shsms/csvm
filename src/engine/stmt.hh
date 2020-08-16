#ifndef CSVQ_STMT_H
#define CSVQ_STMT_H

#include "../models/models.hh"
#include <string>

namespace engine {

class stmt {
  public:
    virtual void add_ident(const std::string &) = 0;
    virtual models::header_row set_header(const models::header_row &) = 0;

    virtual void add_str(const std::string &) {
        throw std::runtime_error("stmt::add_str not implemented");
    }

    virtual void add_num(const std::string &) {
        throw std::runtime_error("stmt::add_num not implemented");
    }

    virtual void add_bang() {
        throw std::runtime_error("stmt::add_bang not implemented");
    }

    virtual void add_oper(const std::string &) {
        throw std::runtime_error("stmt::add_oper not implemented");
    }

    virtual void finalize() {}

    virtual std::string string() = 0;

    virtual bool apply(models::row &) = 0;
};

} // namespace engine
#endif
