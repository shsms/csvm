#ifndef CSVM_MERGESTMT_HH
#define CSVM_MERGESTMT_HH

#include "sortstmt.hh"

namespace engine {

class merge_worker {
  private:
    int curr_pos{};
    std::vector<sortspec> columns;

    template <typename Collector>
    void merge_and_collect(std::vector<merge_chunk> &chunks, Collector &collector);

  public:
    merge_worker(const std::vector<sortspec> &cols) : columns(cols) {}

    bool run(threading::queue<merge_chunk> &in_queue, threading::bin_queue &merged);
};

} // namespace engine

#endif /* CSVM_MERGESTMT_HH */
