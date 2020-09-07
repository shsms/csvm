#ifndef CSVM_MERGESTMT_HH
#define CSVM_MERGESTMT_HH

#include "stmt.hh"
#include "sortstmt.hh"
namespace engine {


class mergestmt : public stmt {
  private:
    int curr_pos{};
    std::vector<sortspec> columns;

  public:
    mergestmt(const std::vector<sortspec> &cols) : columns(cols) {}
    
    void add_ident(const std::string &col) override;
    void
    add_oper(const std::string &oper) override;
    std::string string() override;

    inline stmt::exec_order finalize() override { return sep_block; }

    void set_header(models::header_row &h) override;
    bool apply(models::bin_chunk &chunk,
               std::stack<models::value> &eval_stack) override;
    bool run_worker(threading::bin_queue &, std::function<void(models::bin_chunk&)>) override;
};
    
}

#endif /* CSVM_MERGESTMT_HH */
