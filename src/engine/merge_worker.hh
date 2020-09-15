#ifndef CSVM_MERGESTMT_HH
#define CSVM_MERGESTMT_HH

#include "sortstmt.hh"
#include <future>

namespace engine {

class merge_worker {
  private:
    int curr_pos{};
    std::vector<sortspec> columns;
    int thread_count{1};

    struct merge_task {
        std::vector<merge_chunk> chunks;
        std::promise<merge_chunk> promise;
    };

    std::vector<std::thread> additional_workers;
    threading::queue<merge_task> task_queue;

    template <typename Collector>
    void merge_and_collect(std::vector<merge_chunk> chunks, Collector collector);

  public:
    merge_worker(const std::vector<sortspec> &cols, int thread_count)
        : columns(cols), thread_count(thread_count) {}

    bool run(threading::queue<merge_chunk> &in_queue, threading::bin_queue &merged);
};

} // namespace engine

#endif /* CSVM_MERGESTMT_HH */
