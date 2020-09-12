#ifndef CSVM_MERGESTMT_HH
#define CSVM_MERGESTMT_HH

#include "sortstmt.hh"
#include "stmt.hh"
namespace engine {

class mergestmt : public stmt {
  private:
    int curr_pos{};
    std::vector<sortspec> columns;

  public:
    mergestmt(const std::vector<sortspec> &cols) : columns(cols) {}

    void add_ident(const std::string &col) override;
    void add_oper(const std::string &oper) override;
    std::string string() override;

    inline stmt::exec_order finalize() override { return sep_block; }

    void set_header(models::header_row &h) override;
    bool apply(models::bin_chunk &chunk,
               std::stack<models::value> &eval_stack) override;
    bool
    run_merge_worker(threading::queue<merge_chunk> &in_queue,
                     const std::function<void(models::bin_chunk &)> &forwarder);
};

} // namespace engine

#endif /* CSVM_MERGESTMT_HH */
