#ifndef CSVM_SORTSTMT_HH
#define CSVM_SORTSTMT_HH

#include "../barrier.hh"
#include "stmt.hh"
#include <algorithm>
#include <fmt/format.h>
#include <stack>
#include <stdexcept>

namespace engine {

struct sortspec {
    bool descending;
    bool numeric;
    int pos;
    std::string name;
};

struct merge_row {
    int chunk_id;
    int chunk_pos;
    models::row m_row;
};

using merge_chunk = std::vector<merge_row>;

class sortstmt : public stmt {
  private:
    int curr_pos{};
    std::vector<sortspec> columns;

    static std::atomic<bool> merge_thread_created;
    static std::thread merge_thread;
    static threading::queue<merge_chunk> to_merge;
    static threading::barrier barrier;

  public:
    void add_ident(const std::string &col) override;
    void add_oper(const std::string &oper) override;
    std::string string() override;

    inline stmt::exec_order finalize() override { return sep_block; }

    void set_header(models::header_row & /*unused*/) override;
    void set_thread_count(int /*unused*/) override;
    bool apply(models::bin_chunk & /*chunk*/,
               std::stack<models::value> & /*eval_stack*/) override;
    bool run_worker(
        threading::bin_queue & /*in_queue*/,
        const std::function<void(models::bin_chunk &)> & /*unused*/) override;
};

} // namespace engine

#endif /* CSVM_SORTSTMT_HH */
