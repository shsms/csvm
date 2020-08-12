#ifndef CSVQ_STMT_H
#define CSVQ_STMT_H

#include "../models/models.hh"
#include <string>

namespace engine {

class stmt {
  public:
    virtual void add_ident(const std::string &) = 0;
    virtual std::string string() = 0;
    virtual void add_bang() = 0;
    virtual models::row set_header(const models::row &) = 0;
    virtual models::row apply(const models::row &) = 0;
    // virtual void apply(); // TODO: should take row and return row
};

} // namespace engine
#endif
